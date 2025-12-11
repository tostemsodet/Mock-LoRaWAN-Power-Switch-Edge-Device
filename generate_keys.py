#!/usr/bin/env python3
"""
LoRaWAN Key Generator
Generates random Device Address, Network Session Key, and Application Session Key
for ABP (Activation By Personalization) mode.
"""

import secrets
import sys

def generate_hex_array(length):
    """Generate a random hex array of specified length"""
    return [secrets.randbelow(256) for _ in range(length)]

def format_c_array(arr, name):
    """Format array as C/C++ style array declaration"""
    hex_values = ', '.join(f'0x{b:02X}' for b in arr)
    return f"static const PROGMEM u1_t {name}[{len(arr)}] = {{ {hex_values} }};"

def generate_devaddr():
    """Generate a random Device Address (4 bytes)"""
    # Generate 4 random bytes
    addr_bytes = generate_hex_array(4)
    # Format as hex number
    devaddr = (addr_bytes[0] << 24) | (addr_bytes[1] << 16) | (addr_bytes[2] << 8) | addr_bytes[3]
    return devaddr, addr_bytes

def main():
    print("=" * 70)
    print("LoRaWAN ABP Keys Generator")
    print("=" * 70)
    print()

    # Generate Device Address
    devaddr, devaddr_bytes = generate_devaddr()

    # Generate Network Session Key (16 bytes)
    nwkskey = generate_hex_array(16)

    # Generate Application Session Key (16 bytes)
    appskey = generate_hex_array(16)

    # Display in multiple formats
    print("Device Address (DEVADDR):")
    print(f"  Hex: 0x{devaddr:08X}")
    print(f"  MSB: {' '.join(f'{b:02X}' for b in devaddr_bytes)}")
    print(f"  LSB: {' '.join(f'{b:02X}' for b in reversed(devaddr_bytes))}")
    print()

    print("Network Session Key (NWKSKEY) - MSB:")
    print(f"  {' '.join(f'{b:02X}' for b in nwkskey)}")
    print()

    print("Application Session Key (APPSKEY) - MSB:")
    print(f"  {' '.join(f'{b:02X}' for b in appskey)}")
    print()

    print("-" * 70)
    print("For Arduino/PlatformIO (Copy to main.cpp):")
    print("-" * 70)
    print(f"static const u4_t DEVADDR = 0x{devaddr:08X};")
    print(format_c_array(nwkskey, "NWKSKEY"))
    print(format_c_array(appskey, "APPSKEY"))
    print()

    print("-" * 70)
    print("For The Things Network (TTN) Console:")
    print("-" * 70)
    print(f"Device Address: {' '.join(f'{b:02X}' for b in devaddr_bytes)}")
    print(f"Network Session Key: {' '.join(f'{b:02X}' for b in nwkskey)}")
    print(f"App Session Key: {' '.join(f'{b:02X}' for b in appskey)}")
    print()

    print("-" * 70)
    print("Save these keys securely! You'll need them to:")
    print("1. Update main.cpp with the Arduino format keys")
    print("2. Register your device in TTN Console with the TTN format keys")
    print("-" * 70)
    print()

    # Ask if user wants to save to file
    try:
        response = input("Do you want to save these keys to 'lorawan_keys.txt'? (y/n): ").lower()
        if response == 'y':
            with open('lorawan_keys.txt', 'w') as f:
                f.write("=" * 70 + "\n")
                f.write("LoRaWAN ABP Keys\n")
                f.write("=" * 70 + "\n\n")

                f.write("Arduino/PlatformIO Format:\n")
                f.write("-" * 70 + "\n")
                f.write(f"static const u4_t DEVADDR = 0x{devaddr:08X};\n")
                f.write(format_c_array(nwkskey, "NWKSKEY") + "\n")
                f.write(format_c_array(appskey, "APPSKEY") + "\n\n")

                f.write("The Things Network (TTN) Format:\n")
                f.write("-" * 70 + "\n")
                f.write(f"Device Address: {' '.join(f'{b:02X}' for b in devaddr_bytes)}\n")
                f.write(f"Network Session Key: {' '.join(f'{b:02X}' for b in nwkskey)}\n")
                f.write(f"App Session Key: {' '.join(f'{b:02X}' for b in appskey)}\n")

            print("Keys saved to 'lorawan_keys.txt'")
            print()
    except (KeyboardInterrupt, EOFError):
        print("\nSkipping file save.")
        print()

if __name__ == "__main__":
    main()
