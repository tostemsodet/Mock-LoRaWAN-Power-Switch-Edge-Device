# Mock LoRaWAN Power Switch - Implementation Documentation

This document describes the implementation of a mock power monitoring device using TTGO LoRa32 v1 board.

## Project Overview

A mock power switch device that simulates power monitoring and control over LoRaWAN network. The device connects to The Things Network (TTN) using ABP activation and sends mock power readings using CayenneLPP format.

## Hardware

**Board:** TTGO LoRa32 OLED v1
- **MCU:** ESP32 (240MHz, 320KB RAM, 4MB Flash)
- **LoRa:** SX1276/78 (923MHz for AS923 Thailand)
- **LED:** GPIO 2 (built-in)
- **Button:** GPIO 0 (BOOT button)

## Features Implemented

### 1. Mock Power Switch
- Toggle switch ON/OFF using BOOT button
- Built-in LED indicates switch state (ON = LED lit, OFF = LED off)
- When ON: generates random voltage (220-240V) and current (0.5-5.0A)
- When OFF: all values are 0

### 2. LoRaWAN Communication
- **Mode:** ABP (Activation By Personalization)
- **Band:** AS923 (923MHz - Thailand)
- **Protocol:** LoRaWAN 1.0.3
- **Payload:** CayenneLPP format
- **Transmission:** Every 60 seconds

### 3. Data Channels (CayenneLPP)
- **Channel 1:** Switch state (Digital Output: 0=OFF, 1=ON)
- **Channel 2:** Voltage (Analog Input: volts)
- **Channel 3:** Current (Analog Input: amperes)
- **Channel 4:** Power (Analog Input: watts, calculated as V×I)

### 4. Downlink Control
Remote LED control via LoRaWAN downlink (FPort 1):
- `0x00` - Turn LED OFF
- `0x01` - Turn LED ON
- `0x02` - Toggle LED state

## Project Structure

```
lorawan/
├── platformio.ini                    # Build configuration
├── src/
│   ├── main.cpp                     # Main firmware
│   └── lmic_project_config.h        # LMIC region config
├── include/
│   └── config.h                     # Hardware & LoRaWAN configuration
├── generate_keys.py                 # Key generation script
├── TTN_SETUP.md                     # TTN setup guide
├── README.md                        # Project documentation
├── spec.md                          # Original specification
└── claude.md                        # This file

```

## Configuration Files

### platformio.ini
- Platform: ESP32 (espressif32)
- Board: ttgo-lora32-v1
- Framework: Arduino
- Libraries:
  - MCCI LoRaWAN LMIC library (4.1.1)
  - CayenneLPP
- Build flags: `-D hal_init=lmic_hal_init` (resolves HAL naming conflict)

### include/config.h
Centralized configuration for:
- **LoRaWAN keys** (DEVADDR, NWKSKEY, APPSKEY)
- **Hardware pins** (LED, button, LoRa module)
- **Application settings** (transmission interval, power ranges)
- **AS923 frequencies** (8 channels: 922-923.4 MHz)
- **Downlink commands** (LED control codes)

### src/lmic_project_config.h
LMIC library configuration:
- Region: AS923 enabled
- Radio: SX1276
- Other regions disabled

## Key Components

### Pin Configuration
```cpp
LED_PIN      = 2   // Built-in LED
BOOT_BTN     = 0   // BOOT button
LORA_NSS     = 18  // LoRa SPI chip select
LORA_RST     = 14  // LoRa reset
LORA_DIO0    = 26  // LoRa interrupt 0
LORA_DIO1    = 33  // LoRa interrupt 1
LORA_DIO2    = 32  // LoRa interrupt 2
```

### Main Functions

#### `setup()`
1. Initialize serial communication (115200 baud)
2. Configure LED and button pins
3. Initialize LMIC library
4. Set ABP session parameters
5. Configure AS923 band settings
6. Start first transmission job

#### `loop()`
1. Run LMIC event loop (`os_runloop_once()`)
2. Check button state for manual toggle

#### `do_send()`
1. Generate random power values if switch is ON
2. Encode data using CayenneLPP
3. Queue LoRaWAN uplink transmission
4. Print values to serial monitor

#### `onEvent()`
Handles LoRaWAN events:
- **EV_TXCOMPLETE:** Transmission complete, check for downlink
- **EV_TXSTART:** Transmission started
- **Downlink received:** Parse command and control LED

#### `checkButton()`
Debounced button reading with toggle functionality

## Security

### Key Generation
Use `generate_keys.py` to create secure random keys:
```bash
python3 generate_keys.py
```

Output:
- Device Address (4 bytes)
- Network Session Key (16 bytes)
- Application Session Key (16 bytes)

Keys are provided in two formats:
1. **Arduino format:** For updating `include/config.h`
2. **TTN format:** For TTN Console registration

### Important Security Notes
- Never commit keys to version control
- `lorawan_keys.txt` is in `.gitignore`
- Use OTAA instead of ABP for production
- Generate unique keys per device
- Enable frame counter validation in production

## Build Process

### Dependencies Installation
Libraries are automatically installed by PlatformIO:
- MCCI LoRaWAN LMIC library
- CayenneLPP
- ESP32 Arduino framework

### Compilation
```bash
~/.platformio/penv/bin/pio run
```

### Upload to Board
```bash
~/.platformio/penv/bin/pio run --target upload
```

### Monitor Serial Output
```bash
~/.platformio/penv/bin/pio device monitor
```

## Serial Output Examples

### Startup
```
Starting Mock LoRaWAN Power Switch
LoRaWAN initialized (ABP mode)
AS923 band configured for Thailand
Press BOOT button to toggle switch
```

### Uplink Transmission
```
Sending uplink...
Switch: ON
Voltage: 235.3 V
Current: 2.47 A
Power: 581.2 W
EV_TXSTART
Packet queued
EV_TXCOMPLETE (includes waiting for RX windows)
```

### Button Press
```
Button pressed! Switch state: ON
```

### Downlink Received
```
Received 1 bytes of payload
Downlink command: 0x01
LED turned ON via downlink
```

## The Things Network Integration

### Application Setup
1. Create TTN application
2. Register device with ABP activation
3. Enter DEVADDR, NWKSKEY, APPSKEY
4. Set payload formatter to "Cayenne LPP"
5. Disable frame counter checks (development only)

### Data Format (Decoded)
```json
{
  "digital_out_1": 1,      // Switch state
  "analog_in_2": 235.3,    // Voltage
  "analog_in_3": 2.47,     // Current
  "analog_in_4": 581.2     // Power
}
```

### Sending Downlink
1. Go to device page in TTN Console
2. Navigate to "Downlink" section
3. Set FPort to 1
4. Enter hex payload (00, 01, or 02)
5. Click "Send"
6. Device receives command after next uplink

## Customization

### Change Transmission Interval
Edit `include/config.h`:
```cpp
#define TX_INTERVAL 60  // seconds
```

### Adjust Power Value Ranges
Edit `include/config.h`:
```cpp
#define VOLTAGE_MIN 220.0f
#define VOLTAGE_MAX 240.0f
#define CURRENT_MIN 0.5f
#define CURRENT_MAX 5.0f
```

### Update LoRaWAN Keys
1. Run `python3 generate_keys.py`
2. Copy Arduino format to `include/config.h`
3. Copy TTN format to TTN Console
4. Rebuild and upload firmware

## Memory Usage

**Flash:** 294,765 bytes (22.5% of 1,310,720 bytes)
**RAM:** 22,596 bytes (6.9% of 327,680 bytes)

Plenty of space available for additional features.

## Troubleshooting

### Common Issues

**1. Device not connecting**
- Verify keys match between firmware and TTN Console
- Check gateway coverage (TTN map)
- Ensure AS923 region selected in TTN
- Disable frame counter checks during testing

**2. Build errors**
- Clean build: `pio run --target clean`
- Update libraries: `pio pkg update`
- Check PlatformIO installation

**3. No downlink received**
- Downlinks only work in RX windows after uplink
- Send downlink, wait for next transmission
- Check FPort is set to 1
- Verify gateway supports downlink

**4. Button not working**
- Button is active LOW (pressed = LOW)
- Check GPIO 0 connection
- Look for serial output "Button pressed!"

## Future Enhancements

Possible improvements:
- Add OTAA support (more secure than ABP)
- Implement deep sleep for battery operation
- Add real sensor support (INA219, ACS712)
- Web configuration portal
- MQTT local integration
- Data logging to SD card
- Multiple switch support
- Scheduled operations
- Energy consumption tracking

## References

### Documentation
- [The Things Network](https://www.thethingsnetwork.org/)
- [MCCI LMIC Library](https://github.com/mcci-catena/arduino-lmic)
- [CayenneLPP](https://developers.mydevices.com/cayenne/docs/lora/)
- [AS923 Frequency Plan](https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/)

### Hardware
- [TTGO LoRa32 v1](https://github.com/LilyGO/TTGO-LORA32)
- [ESP32 Datasheet](https://www.espressif.com/en/products/socs/esp32)
- [SX1276 Datasheet](https://www.semtech.com/products/wireless-rf/lora-core/sx1276)

## License

This project is provided as-is for educational and development purposes.

## Development Timeline

**Implementation completed:** 2025-12-11

**Key milestones:**
1. Project structure setup
2. LoRaWAN LMIC integration
3. CayenneLPP payload implementation
4. Button and LED control
5. Downlink command handling
6. Documentation and setup guide
7. OLED display removed (not needed)
8. Build verification and testing

---

**Generated by:** Claude Code Assistant
**Date:** December 11, 2025
