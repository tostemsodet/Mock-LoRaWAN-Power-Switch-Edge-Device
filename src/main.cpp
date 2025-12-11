#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <CayenneLPP.h>
#include "config.h"

// LoRaWAN Keys - Configure in include/config.h
static const u4_t DEVADDR = LORAWAN_DEVADDR;
static u1_t NWKSKEY[16] = LORAWAN_NWKSKEY;
static u1_t APPSKEY[16] = LORAWAN_APPSKEY;

// Pin mapping for TTGO LoRa32 v1
const lmic_pinmap lmic_pins = {
    .nss = LORA_NSS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LORA_RST,
    .dio = {LORA_DIO0, LORA_DIO1, LORA_DIO2},
};

// State variables
bool switchState = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = DEBOUNCE_DELAY;
static osjob_t sendjob;

// Mock power readings
float voltage = 0.0;
float current = 0.0;
float power = 0.0;

// CayenneLPP buffer
CayenneLPP lpp(51);

// Function declarations
void do_send(osjob_t* j);
float getRandomFloat(float min, float max);
void checkButton();

// These callbacks are required by LMIC but not used in ABP mode
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));

              // Handle downlink - control LED
              if (LMIC.dataLen > 0) {
                uint8_t cmd = LMIC.frame[LMIC.dataBeg];
                Serial.print(F("Downlink command: 0x"));
                Serial.println(cmd, HEX);

                // Handle downlink commands
                if (cmd == CMD_LED_OFF) {
                  switchState = false;
                  digitalWrite(LED_PIN, LOW);
                  Serial.println(F("LED turned OFF via downlink"));
                } else if (cmd == CMD_LED_ON) {
                  switchState = true;
                  digitalWrite(LED_PIN, HIGH);
                  Serial.println(F("LED turned ON via downlink"));
                } else if (cmd == CMD_LED_TOGGLE) {
                  switchState = !switchState;
                  digitalWrite(LED_PIN, switchState ? HIGH : LOW);
                  Serial.println(F("LED toggled via downlink"));
                }
              }
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Generate random mock power values
        if (switchState) {
            voltage = getRandomFloat(VOLTAGE_MIN, VOLTAGE_MAX);
            current = getRandomFloat(CURRENT_MIN, CURRENT_MAX);
            power = voltage * current;
        } else {
            voltage = 0.0;
            current = 0.0;
            power = 0.0;
        }

        // Prepare CayenneLPP payload
        lpp.reset();
        lpp.addDigitalOutput(1, switchState ? 1 : 0);  // Channel 1: Switch state
        lpp.addAnalogInput(2, voltage);                 // Channel 2: Voltage
        lpp.addAnalogInput(3, current);                 // Channel 3: Current
        lpp.addAnalogInput(4, power);                   // Channel 4: Power

        // Print values
        Serial.println(F("Sending uplink..."));
        Serial.print(F("Switch: "));
        Serial.println(switchState ? "ON" : "OFF");
        Serial.print(F("Voltage: "));
        Serial.print(voltage);
        Serial.println(F(" V"));
        Serial.print(F("Current: "));
        Serial.print(current);
        Serial.println(F(" A"));
        Serial.print(F("Power: "));
        Serial.print(power);
        Serial.println(F(" W"));

        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, lpp.getBuffer(), lpp.getSize(), 0);
        Serial.println(F("Packet queued"));
    }
}

void checkButton() {
    int reading = digitalRead(BOOT_BTN);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading == LOW) {  // Button pressed (active LOW)
            switchState = !switchState;
            digitalWrite(LED_PIN, switchState ? HIGH : LOW);
            Serial.print(F("Button pressed! Switch state: "));
            Serial.println(switchState ? "ON" : "OFF");
            delay(300);  // Simple debounce delay
        }
    }

    lastButtonState = reading;
}

float getRandomFloat(float min, float max) {
    return min + (float)random(0, 10000) / 10000.0 * (max - min);
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    Serial.println(F("Starting Mock LoRaWAN Power Switch"));

    // Initialize pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(BOOT_BTN, INPUT_PULLUP);
    digitalWrite(LED_PIN, LOW);

    // LMIC init
    os_init();
    LMIC_reset();

    // Set static session parameters for ABP
    LMIC_setSession (0x13, DEVADDR, NWKSKEY, APPSKEY);

    // AS923 channels are configured automatically by the LMIC library
    // No manual channel setup needed

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window in AS923
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power
    LMIC_setDrTxpow(LORA_DR, LORA_TX_POWER);

    Serial.println(F("LoRaWAN initialized (ABP mode)"));
    Serial.println(F("AS923 band configured for Thailand"));
    Serial.println(F("Press BOOT button to toggle switch"));

    // Start job
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
    checkButton();
}
