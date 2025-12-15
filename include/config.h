#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// LoRaWAN Configuration
// ============================================================================
// Keys are auto-generated during build process
// See include/generated_keys.h (created automatically)
// To regenerate keys: run 'pio run -t clean' then rebuild
// ============================================================================

#include "generated_keys.h"

// ============================================================================
// Hardware Configuration
// ============================================================================

// TTGO LoRa32 v1 Pin Definitions
#define LED_PIN         2       // Built-in LED
#define BOOT_BTN        0       // BOOT button
#define LORA_NSS        18      // LoRa NSS
#define LORA_RST        14      // LoRa Reset
#define LORA_DIO0       26      // LoRa DIO0
#define LORA_DIO1       33      // LoRa DIO1
#define LORA_DIO2       32      // LoRa DIO2

// OLED Display (SSD1306) Pins
#define OLED_SDA        4       // I2C Data
#define OLED_SCL        15      // I2C Clock
#define OLED_RST        16      // Reset

// ============================================================================
// Application Configuration
// ============================================================================

// Transmission interval in seconds
#define TX_INTERVAL     60

// Mock power reading ranges
#define VOLTAGE_MIN     220.0f
#define VOLTAGE_MAX     240.0f
#define CURRENT_MIN     0.5f
#define CURRENT_MAX     5.0f

// Button debounce settings
#define DEBOUNCE_DELAY  50      // milliseconds

// Serial baud rate
#define SERIAL_BAUD     115200

// ============================================================================
// LoRaWAN Band Configuration (AS923 - Thailand)
// ============================================================================

// AS923 Frequency Plan
#define LORA_CHANNEL_0  923200000  // 923.2 MHz
#define LORA_CHANNEL_1  923400000  // 923.4 MHz
#define LORA_CHANNEL_2  922000000  // 922.0 MHz
#define LORA_CHANNEL_3  922200000  // 922.2 MHz
#define LORA_CHANNEL_4  922400000  // 922.4 MHz
#define LORA_CHANNEL_5  922600000  // 922.6 MHz
#define LORA_CHANNEL_6  922800000  // 922.8 MHz
#define LORA_CHANNEL_7  923000000  // 923.0 MHz

// Data rate and TX power
#define LORA_DR         DR_SF7     // Spreading Factor 7
#define LORA_TX_POWER   14         // dBm

// ============================================================================
// Downlink Command Definitions
// ============================================================================

#define CMD_LED_OFF     0x00
#define CMD_LED_ON      0x01
#define CMD_LED_TOGGLE  0x02

#endif // CONFIG_H
