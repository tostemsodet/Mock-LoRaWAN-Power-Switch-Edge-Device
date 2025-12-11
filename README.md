# Mock LoRaWAN Power Switch

A mock power monitoring device using TTGO LoRa32 OLED v1 board that simulates a power switch with voltage, current, and power measurements over LoRaWAN.

## Features

- **Built-in LED**: Visual indicator for switch state
- **BOOT Button**: Manual toggle switch control
- **Mock Power Readings**: Random voltage (220-240V), current (0.5-5.0A), and calculated power
- **LoRaWAN Connectivity**: Connects to The Things Network (TTN)
- **AS923 Band**: Configured for Thailand (923MHz)
- **CayenneLPP**: Standard payload format for easy integration
- **Downlink Support**: Remote LED control via LoRaWAN

## Hardware

- **Board**: TTGO LoRa32 OLED v1 (ESP32 + SX1276/78)
- **Frequency**: 923MHz (AS923)
- **Antenna**: Built-in or external

## Quick Start

### 1. Generate Keys

```bash
python3 generate_keys.py
```

Save the generated keys securely.

### 2. Update Firmware

Edit `include/config.h` and replace the placeholder keys with your generated keys:

```cpp
#define LORAWAN_DEVADDR 0xYOUR_DEVADDR
#define LORAWAN_NWKSKEY { 0xAA, 0xBB, ... }
#define LORAWAN_APPSKEY { 0x11, 0x22, ... }
```

### 3. Configure TTN

Follow the detailed instructions in [TTN_SETUP.md](TTN_SETUP.md) to:
- Create an application
- Register your device (ABP mode)
- Configure payload decoder

### 4. Build and Upload

```bash
pio run --target upload
pio device monitor
```

## Usage

### Manual Control
- Press the **BOOT button** to toggle the switch ON/OFF
- LED will indicate current state (ON = lit, OFF = off)
- Monitor status via serial output (115200 baud)

### Remote Control (Downlink)
Send hex commands via TTN console (FPort 1):
- `00` = Turn OFF
- `01` = Turn ON
- `02` = Toggle

### Data Transmission
Device sends data every 60 seconds (configurable in `include/config.h`):
- Channel 1: Switch state (0=OFF, 1=ON)
- Channel 2: Voltage (V)
- Channel 3: Current (A)
- Channel 4: Power (W)

## CayenneLPP Payload

The device uses CayenneLPP encoding for easy integration with TTN and dashboards:

```
Channel 1: Digital Output (Switch State)
Channel 2: Analog Input (Voltage)
Channel 3: Analog Input (Current)
Channel 4: Analog Input (Power)
```

### TTN Payload Decoder

#### Option 1: Built-in Cayenne LPP (Recommended)

In TTN Console v3:
1. Go to your Application → Payload formatters
2. Select **"Cayenne LPP"** from the Formatter type dropdown
3. Click **Save**

The payload will be automatically decoded to:
```json
{
  "digital_out_1": 1,
  "analog_in_2": 235.3,
  "analog_in_3": 2.47,
  "analog_in_4": 581.2
}
```

#### Option 2: Custom JavaScript Decoder

If you need custom field names, use this decoder:

```javascript
function decodeUplink(input) {
  var data = {};
  var bytes = input.bytes;

  // Parse CayenneLPP format manually
  var i = 0;
  while (i < bytes.length) {
    var channel = bytes[i++];
    var type = bytes[i++];

    switch (type) {
      case 0x00: // Digital Output
        data.switchState = bytes[i++];
        break;
      case 0x02: // Analog Input
        var value = (bytes[i] << 8) | bytes[i + 1];
        if (value > 32767) value -= 65536; // Handle negative
        value = value / 100.0;

        if (channel === 2) data.voltage = value;
        else if (channel === 3) data.current = value;
        else if (channel === 4) data.power = value;
        i += 2;
        break;
      default:
        i += 2; // Skip unknown types
    }
  }

  return {
    data: {
      switchState: data.switchState === 1 ? "ON" : "OFF",
      voltage: data.voltage ? data.voltage + " V" : "0 V",
      current: data.current ? data.current + " A" : "0 A",
      power: data.power ? data.power + " W" : "0 W"
    }
  };
}
```

**Decoded Output:**
```json
{
  "switchState": "ON",
  "voltage": "235.3 V",
  "current": "2.47 A",
  "power": "581.2 W"
}
```

#### Option 3: Simple Decoder (Numeric Values Only)

For basic numeric output:

```javascript
function decodeUplink(input) {
  // CayenneLPP parsing
  var decoded = {};
  var bytes = input.bytes;
  var i = 0;

  while (i < bytes.length) {
    var channel = bytes[i++];
    var type = bytes[i++];

    if (type === 0x00) { // Digital Output
      decoded['ch' + channel] = bytes[i++];
    } else if (type === 0x02) { // Analog Input
      var val = (bytes[i] << 8) | bytes[i + 1];
      if (val > 32767) val -= 65536;
      decoded['ch' + channel] = val / 100.0;
      i += 2;
    } else {
      i += 2;
    }
  }

  return {
    data: {
      switch: decoded.ch1,
      voltage: decoded.ch2,
      current: decoded.ch3,
      power: decoded.ch4
    }
  };
}
```

**Decoded Output:**
```json
{
  "switch": 1,
  "voltage": 235.3,
  "current": 2.47,
  "power": 581.2
}
```

### Raw Payload Example

When switch is ON:
```
01 00 01 02 02 5C 05 03 02 00 F7 04 02 16 AD
```

Breakdown:
- `01 00 01` → Channel 1, Digital Output, Value 1 (ON)
- `02 02 5C 05` → Channel 2, Analog Input, Value 23557 (235.57V)
- `03 02 00 F7` → Channel 3, Analog Input, Value 247 (2.47A)
- `04 02 16 AD` → Channel 4, Analog Input, Value 5805 (58.05W)

When switch is OFF:
```
01 00 00 02 02 00 00 03 02 00 00 04 02 00 00
```

All analog values are 0.

## Configuration

Edit `include/config.h` to customize:

- **TX_INTERVAL**: Transmission interval in seconds (default: 60)
- **VOLTAGE_MIN/MAX**: Voltage range (default: 220-240V)
- **CURRENT_MIN/MAX**: Current range (default: 0.5-5.0A)
- **LED_PIN**: Built-in LED GPIO (default: 2)
- **BOOT_BTN**: Button GPIO (default: 0)

## Project Structure

```
lorawan/
├── platformio.ini              # PlatformIO configuration
├── src/
│   ├── main.cpp               # Main firmware
│   └── lmic_project_config.h  # LMIC region config
├── include/
│   └── config.h               # Hardware & LoRaWAN config
├── generate_keys.py           # Key generation script
├── TTN_SETUP.md              # Complete TTN setup guide
├── README.md                 # This file
├── spec.md                   # Original specification
└── claude.md                 # Implementation docs
```

## Documentation

- [TTN_SETUP.md](TTN_SETUP.md) - Complete setup guide for The Things Network
- [claude.md](claude.md) - Detailed implementation documentation
- [spec.md](spec.md) - Original project specification

## Dependencies

All dependencies are automatically installed via PlatformIO:

- MCCI LoRaWAN LMIC library ^4.1.1
- CayenneLPP (from GitHub)

## Troubleshooting

See the [Troubleshooting section](TTN_SETUP.md#troubleshooting) in TTN_SETUP.md for common issues and solutions.

## Security

- Never commit `lorawan_keys.txt` to version control
- Generate unique keys for each device
- Use OTAA instead of ABP for production
- Enable frame counter validation in production

## License

This project is provided as-is for educational and development purposes.
