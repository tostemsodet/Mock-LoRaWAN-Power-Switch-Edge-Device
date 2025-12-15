#!/usr/bin/env python3
"""
Pre-build script to auto-generate LoRaWAN keys
Generates random Device Address, Network Session Key, and Application Session Key
"""

Import("env")
import secrets
import os

def generate_hex_array(length):
    """Generate a random hex array of specified length"""
    return [secrets.randbelow(256) for _ in range(length)]

def format_c_array(arr, name):
    """Format array as C/C++ style array declaration"""
    hex_values = ', '.join(f'0x{b:02X}' for b in arr)
    return f"{{ {hex_values} }}"

def generate_devaddr():
    """Generate a random Device Address (4 bytes)"""
    addr_bytes = generate_hex_array(4)
    devaddr = (addr_bytes[0] << 24) | (addr_bytes[1] << 16) | (addr_bytes[2] << 8) | addr_bytes[3]
    return devaddr, addr_bytes

def generate_keys(source, target, env):
    """Generate LoRaWAN keys and create header file"""

    print("=" * 70)
    print("Auto-generating LoRaWAN Keys...")
    print("=" * 70)

    # Generate keys
    devaddr, devaddr_bytes = generate_devaddr()
    nwkskey = generate_hex_array(16)
    appskey = generate_hex_array(16)

    # Print to console
    print(f"\nDevice Address (DEVADDR): 0x{devaddr:08X}")
    print(f"  MSB: {' '.join(f'{b:02X}' for b in devaddr_bytes)}")

    print(f"\nNetwork Session Key (NWKSKEY):")
    print(f"  MSB: {' '.join(f'{b:02X}' for b in nwkskey)}")

    print(f"\nApplication Session Key (APPSKEY):")
    print(f"  MSB: {' '.join(f'{b:02X}' for b in appskey)}")

    # Create header file
    header_content = f"""#ifndef GENERATED_KEYS_H
#define GENERATED_KEYS_H

// AUTO-GENERATED FILE - DO NOT EDIT MANUALLY
// Generated during build process
// To regenerate: clean and rebuild the project

// Device Address (4 bytes) - MSB format
#define LORAWAN_DEVADDR 0x{devaddr:08X}

// Network Session Key (16 bytes) - MSB format
#define LORAWAN_NWKSKEY {format_c_array(nwkskey, "NWKSKEY")}

// Application Session Key (16 bytes) - MSB format
#define LORAWAN_APPSKEY {format_c_array(appskey, "APPSKEY")}

// For The Things Network (TTN) Console - copy these values:
// Device Address: {' '.join(f'{b:02X}' for b in devaddr_bytes)}
// Network Session Key: {' '.join(f'{b:02X}' for b in nwkskey)}
// App Session Key: {' '.join(f'{b:02X}' for b in appskey)}

#endif // GENERATED_KEYS_H
"""

    # Write header file
    output_path = os.path.join("include", "generated_keys.h")
    with open(output_path, 'w') as f:
        f.write(header_content)

    print(f"\n{'-' * 70}")
    print(f"Keys saved to: {output_path}")
    print(f"Keys will be displayed at device startup on serial monitor.")
    print(f"{'-' * 70}")

    # Also save to a text file for TTN registration
    keys_file = "lorawan_keys.txt"
    with open(keys_file, 'w') as f:
        f.write("=" * 70 + "\n")
        f.write("LoRaWAN ABP Keys (Auto-Generated)\n")
        f.write("=" * 70 + "\n\n")

        f.write("For The Things Network (TTN) Console:\n")
        f.write("-" * 70 + "\n")
        f.write(f"Device Address: {' '.join(f'{b:02X}' for b in devaddr_bytes)}\n")
        f.write(f"Network Session Key: {' '.join(f'{b:02X}' for b in nwkskey)}\n")
        f.write(f"App Session Key: {' '.join(f'{b:02X}' for b in appskey)}\n\n")

        f.write("Device will display these keys on serial monitor at startup.\n")

    print(f"TTN registration info saved to: {keys_file}")
    print("=" * 70)
    print()

# Register the pre-build script
env.AddPreAction("buildprog", generate_keys)
