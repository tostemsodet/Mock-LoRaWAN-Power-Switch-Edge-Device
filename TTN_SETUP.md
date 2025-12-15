# The Things Network Setup Guide

Complete guide for setting up your TTGO LoRa32 v1 Mock Power Switch with The Things Network.

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Generate LoRaWAN Keys](#generate-lorawan-keys)
3. [Configure The Things Network](#configure-the-things-network)
4. [Update Firmware with Keys](#update-firmware-with-keys)
5. [Upload and Test](#upload-and-test)
6. [Sending Downlink Commands](#sending-downlink-commands)
7. [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Hardware
- TTGO LoRa32 OLED v1 board
- USB cable for programming
- LoRaWAN gateway in range (or TTN community gateway)

### Software
- PlatformIO (installed via VSCode or CLI)
- Python 3.x (for key generation)
- The Things Network account (https://www.thethingsnetwork.org/)

### TTN Account Setup
1. Go to https://www.thethingsnetwork.org/
2. Create an account if you don't have one
3. Log in to the console

---

## Generate LoRaWAN Keys

First, generate secure random keys for your device:

```bash
cd /path/to/lorawan
python3 generate_keys.py
```

The script will output:
- **Device Address (DEVADDR)**: 4-byte device identifier
- **Network Session Key (NWKSKEY)**: 16-byte encryption key
- **Application Session Key (APPSKEY)**: 16-byte encryption key

**IMPORTANT**: Save these keys securely! You'll need them for both your device firmware and TTN configuration.

Example output:
```
Device Address (DEVADDR):
  Hex: 0x26011234
  MSB: 26 01 12 34

Network Session Key (NWKSKEY) - MSB:
  AA BB CC DD EE FF 00 11 22 33 44 55 66 77 88 99

Application Session Key (APPSKEY) - MSB:
  11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF 00
```

---

## Configure The Things Network

### Step 1: Create an Application

1. Log in to [The Things Network Console](https://console.thethingsnetwork.org/)
2. Click on **"Applications"** in the top menu
3. Click **"+ Add application"**
4. Fill in the form:
   - **Application ID**: Choose a unique ID (e.g., `mock-power-switch`)
   - **Description**: "Mock LoRaWAN Power Switch"
   - **Handler registration**: Choose your region (Asia Pacific for Thailand)
5. Click **"Add application"**

### Step 2: Register Your Device (ABP Mode)

1. In your application page, click **"Register device"**
2. Fill in the device registration form:
   - **Device ID**: Choose a unique name (e.g., `ttgo-power-switch-01`)
   - **Device EUI**: Click the **"🔄"** icon to generate one (not used in ABP but required)
   - **Activation method**: Select **"ABP"** (Activation By Personalization)

3. Click **"Register"**

### Step 3: Configure Device Settings

After registration, you'll see the device overview page.

1. **Set Device Address**:
   - Paste your DEVADDR from the key generator (e.g., `26011234`)
   - Format: MSB (Most Significant Byte first)

2. **Set Network Session Key**:
   - Paste your NWKSKEY from the key generator
   - Format: MSB (Most Significant Byte first)
   - Example: `AA BB CC DD EE FF 00 11 22 33 44 55 66 77 88 99`

3. **Set App Session Key**:
   - Paste your APPSKEY from the key generator
   - Format: MSB (Most Significant Byte first)
   - Example: `11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF 00`

4. **Frame Counter Settings** (Important!):
   - Scroll down to **"Settings"**
   - **Disable frame counter checks** during development:
     - Uncheck "Frame counter width" or set to 32-bit
     - This prevents issues when resetting the device
   - **For production**: Enable frame counter checks for security

5. Click **"Save"**

### Step 4: Configure Payload Decoder (CayenneLPP)

1. In your application page, click on **"Payload Formats"**
2. Select **"Cayenne LPP"** from the dropdown
3. Click **"Save payload functions"**

This will automatically decode the data sent by your device:
- Channel 1: Switch state (digital output)
- Channel 2: Voltage (analog input)
- Channel 3: Current (analog input)
- Channel 4: Power (analog input)

---

## Update Firmware with Keys

Now update your Arduino code with the generated keys:

1. Open `src/main.cpp` in your editor
2. Find the following lines (around line 13-15):

```cpp
static const u4_t DEVADDR = 0x26011234;  // Device Address (MSB)
static const PROGMEM u1_t NWKSKEY[16] = { 0x00, 0x00, ... };
static const PROGMEM u1_t APPSKEY[16] = { 0x00, 0x00, ... };
```

3. Replace them with your generated keys from `generate_keys.py`:

```cpp
static const u4_t DEVADDR = 0xYOUR_DEVADDR;
static const PROGMEM u1_t NWKSKEY[16] = { 0xAA, 0xBB, 0xCC, ... };
static const PROGMEM u1_t APPSKEY[16] = { 0x11, 0x22, 0x33, ... };
```

4. Save the file

---

## Upload and Test

### Build and Upload Firmware

```bash
# Build the project
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor
```

### What to Expect

1. **Initial Boot**:
   ```
   Starting Mock LoRaWAN Power Switch
   LoRaWAN initialized (ABP mode)
   AS923 band configured for Thailand
   Press BOOT button to toggle switch
   ```

2. **First Uplink**:
   ```
   Sending uplink...
   Switch: OFF
   Voltage: 0.0 V
   Current: 0.0 A
   Power: 0.0 W
   EV_TXSTART
   Packet queued
   EV_TXCOMPLETE (includes waiting for RX windows)
   ```

3. **OLED Display** shows:
   - Title: "Mock Power Switch"
   - State: ON/OFF
   - Voltage, Current, Power readings

### View Data on TTN Console

1. Go to your device page in TTN Console
2. Click on the **"Data"** tab
3. You should see incoming messages every 60 seconds
4. Data will be automatically decoded in Cayenne LPP format:
   ```json
   {
     "digital_out_1": 0,
     "analog_in_2": 0.0,
     "analog_in_3": 0.0,
     "analog_in_4": 0.0
   }
   ```

### Test the Button

1. Press the **BOOT button** on the TTGO board
2. The built-in LED should turn ON
3. OLED display should update to show "State: ON"
4. Next uplink will show:
   - Voltage: 220-240V (random)
   - Current: 0.5-5.0A (random)
   - Power: V × I (calculated)

---

## Sending Downlink Commands

You can remotely control the LED/switch state via LoRaWAN downlink.

### Downlink Command Format

Send a single byte as payload (port 1):
- `0x00` = Turn switch OFF
- `0x01` = Turn switch ON
- `0x02` = Toggle switch state

### Setup Downlink Encoder (Recommended)

To use JSON format instead of hex, configure a downlink encoder:

1. Go to your Application → **Payload formatters**
2. Under **"Downlink formatter"**, select **"Custom JavaScript formatter"**
3. Paste this code:

```javascript
function encodeDownlink(input) {
  var bytes = [];

  if (input.data.switch !== undefined) {
    // Switch command: 0 = OFF, 1 = ON
    bytes.push(input.data.switch ? 0x01 : 0x00);
  } else if (input.data.toggle) {
    // Toggle command
    bytes.push(0x02);
  }

  return {
    bytes: bytes,
    fPort: 1,
    warnings: [],
    errors: []
  };
}

function decodeDownlink(input) {
  var data = {};

  if (input.bytes.length > 0) {
    var cmd = input.bytes[0];

    if (cmd === 0x00) {
      data.command = "OFF";
      data.switch = 0;
    } else if (cmd === 0x01) {
      data.command = "ON";
      data.switch = 1;
    } else if (cmd === 0x02) {
      data.command = "TOGGLE";
      data.toggle = true;
    } else {
      data.command = "UNKNOWN";
      data.raw = cmd;
    }
  }

  return {
    data: data,
    warnings: [],
    errors: []
  };
}
```

4. Click **"Save"**

### Send Downlink via TTN Console

**With Downlink Encoder (JSON):**
1. Go to your device page in TTN Console
2. Scroll to **"Messaging"** → **"Downlink"**
3. Select **"JSON"** format
4. Enter one of:
   - `{"switch": 1}` to turn ON
   - `{"switch": 0}` to turn OFF
   - `{"toggle": true}` to toggle
5. Click **"Schedule downlink"**

**Without Encoder (Raw Hex):**
1. Go to your device page in TTN Console
2. Scroll down to **"Downlink"** section
3. Fill in:
   - **FPort**: `1`
   - **Payload**: Enter hex value (e.g., `01` to turn switch on)
   - **Confirmed**: Leave unchecked (or check for ACK)
4. Click **"Send"**

The device will receive the command during the next RX window (after an uplink).

### Expected Serial Output

```
EV_TXCOMPLETE (includes waiting for RX windows)
Received 1 bytes of payload
Downlink command: 0x01
LED turned ON via downlink
```

---

## Troubleshooting

### Device Not Connecting

**Problem**: No data appearing in TTN Console

**Solutions**:
1. **Check Keys**: Verify DEVADDR, NWKSKEY, and APPSKEY match exactly
2. **Check Gateway**: Ensure you have a TTN gateway in range
3. **Check Region**: Confirm AS923 frequency plan is set in TTN
4. **Serial Monitor**: Check for error messages
5. **Frame Counters**: Disable frame counter checks in TTN settings during testing

### Serial Monitor Shows "OP_TXRXPEND, not sending"

**Problem**: Previous transmission still in progress

**Solution**: This is normal if transmissions overlap. Wait for completion.

### No Downlink Received

**Problem**: Downlink sent but not received by device

**Solutions**:
1. Downlinks are only received after an uplink (in RX windows)
2. Wait for device to send data, then check serial monitor
3. Ensure downlink is sent to correct FPort (1)
4. Check gateway supports downlink (some don't)

### Keys Not Working

**Problem**: Device registered but not communicating

**Solutions**:
1. Regenerate keys using `generate_keys.py`
2. Update both main.cpp and TTN console with new keys
3. Ensure using MSB (Most Significant Byte) format in TTN
4. Re-upload firmware after changing keys

### Gateway Issues

**Problem**: No nearby gateway

**Solutions**:
1. Check TTN coverage map: https://www.thethingsnetwork.org/map
2. Deploy your own gateway (recommended for development)
3. Use a portable/mobile setup to find coverage
4. Consider using Helium network or other LoRaWAN networks

### OLED Display Not Working

**Problem**: Display stays blank

**Solutions**:
1. Check I2C connections (should be built-in on TTGO LoRa32 v1)
2. Verify display address is 0x3c
3. Check library version compatibility
4. Try different OLED initialization parameters

---

## Hardware Specifications

### TTGO LoRa32 v1 Pinout

- **LoRa Module**: SX1276/78
  - NSS: GPIO 18
  - RST: GPIO 14
  - DIO0: GPIO 26
  - DIO1: GPIO 33
  - DIO2: GPIO 32

- **OLED Display**: SSD1306 128x64
  - SDA: GPIO 4
  - SCL: GPIO 15
  - RST: GPIO 16

- **Other**:
  - Built-in LED: GPIO 2
  - BOOT Button: GPIO 0

### Frequency Plan: AS923 (Thailand)

- **Channels**:
  - CH0: 923.2 MHz
  - CH1: 923.4 MHz
  - CH2: 922.0 MHz
  - CH3: 922.2 MHz
  - CH4: 922.4 MHz
  - CH5: 922.6 MHz
  - CH6: 922.8 MHz
  - CH7: 923.0 MHz

- **Data Rate**: SF7BW125 (default)
- **TX Power**: 14 dBm

---

## Additional Resources

- **The Things Network**: https://www.thethingsnetwork.org/
- **TTN Documentation**: https://www.thethingsnetwork.org/docs/
- **Cayenne LPP**: https://developers.mydevices.com/cayenne/docs/lora/
- **MCCI LMIC Library**: https://github.com/mcci-catena/arduino-lmic
- **TTGO LoRa32**: https://github.com/LilyGO/TTGO-LORA32

---

## Project Structure

```
lorawan/
├── platformio.ini          # PlatformIO configuration
├── src/
│   └── main.cpp           # Main firmware
├── generate_keys.py       # Key generation script
├── TTN_SETUP.md          # This file
└── spec.md               # Project specification
```

---

## Security Notes

1. **Never commit keys to version control**: Add `lorawan_keys.txt` to `.gitignore`
2. **Use OTAA in production**: ABP is easier for development but OTAA is more secure
3. **Enable frame counters**: In production, always enable frame counter validation
4. **Rotate keys periodically**: Generate new keys for production deployments
5. **Secure your TTN account**: Use strong passwords and 2FA

---

## License

This project is provided as-is for educational and development purposes.
