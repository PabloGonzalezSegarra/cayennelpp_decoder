# Cayene Decoder

A modern C++26 library for decoding Cayene LPP (Low Power Payload) format, commonly used in LoRaWAN sensor networks.

## Features

- **Full Cayene LPP v1 Support**: Decode all standard sensor types
- **Custom Type Registration**: Extend with your own sensor types
- **Modern C++26**: Uses `std::expected`, `std::span`, and other modern features
- **Type-Safe**: Strong typing with proper error handling
- **Zero Dependencies at Runtime**: Only nlohmann/json for JSON output

## Supported Data Types

| Type ID | Name | Size | Resolution |
|---------|------|------|------------|
| 0x00 | Digital Input | 1 byte | 1 |
| 0x01 | Digital Output | 1 byte | 1 |
| 0x02 | Analog Input | 2 bytes | 0.01 |
| 0x03 | Analog Output | 2 bytes | 0.01 |
| 0x65 | Luminosity | 2 bytes | 1 Lux |
| 0x66 | Presence | 1 byte | 1 |
| 0x67 | Temperature | 2 bytes | 0.1 °C |
| 0x68 | Humidity | 2 bytes | 0.1 % |
| 0x71 | Accelerometer | 6 bytes | 0.001 G |
| 0x73 | Barometer | 2 bytes | 0.1 hPa |
| 0x86 | Gyrometer | 6 bytes | 0.01 °/s |
| 0x88 | GPS | 9 bytes | Lat/Lon: 0.0001°, Alt: 0.01m |

## Requirements

- **Compiler**: Clang 18+ (with C++26 support)
- **Build System**: CMake 3.25+
- **Build Tool**: Ninja (recommended)
- **Dependencies**: Automatically fetched via FetchContent

## Quick Start

### Building with CMake Presets (Recommended)

```bash
# Configure (Release)
cmake --preset release

# Build
cmake --build build/release

# Run tests
ctest --preset release

# Run example
./build/release/examples/basic_example
```

### Building Manually

```bash
cmake -B build -S . \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja

cmake --build build
ctest --test-dir build --output-on-failure
```

## Usage

### Basic Decoding

```cpp
#include <cayene/decoder.hpp>
#include <print>
#include <vector>

int main() {
    cayene::Decoder decoder;
    
    // Example payload: Temperature (27.2°C) + Humidity (60.0%)
    std::vector<std::uint8_t> payload = {
        0x01, 0x67, 0x01, 0x10,  // Channel 1, Temperature: 272 -> 27.2°C
        0x02, 0x68, 0x02, 0x58   // Channel 2, Humidity: 600 -> 60.0%
    };
    
    auto result = decoder.decode(payload);
    
    if (result) {
        std::println("{}", result.value().dump(2));
        // Output:
        // {
        //   "Temperature_1": 27.2,
        //   "Humidity_2": 60.0
        // }
    } else {
        std::println("Error: {}", static_cast<int>(result.error()));
    }
    
    return 0;
}
```

### Registering Custom Types

```cpp
#include <cayene/decoder.hpp>

int main() {
    cayene::Decoder decoder;
    
    // Register a custom 2-byte battery voltage sensor
    decoder.add_custom_type(
        0xFE,           // Type ID (choose unused ID)
        "Battery",      // Name for JSON key
        2,              // Size in bytes
        [](std::span<const std::uint8_t> data) -> cayene::Json {
            // Decode as millivolts, convert to volts
            auto millivolts = static_cast<std::uint16_t>(
                (static_cast<unsigned>(data[0]) << 8U) |
                static_cast<unsigned>(data[1]));
            return cayene::Json(millivolts / 1000.0);
        }
    );
    
    // Now payloads with type 0xFE will be decoded
    std::vector<std::uint8_t> payload = {
        0x01, 0xFE, 0x0C, 0xE4  // Channel 1, Battery: 3300mV -> 3.3V
    };
    
    auto result = decoder.decode(payload);
    // result.value()["Battery_1"] == 3.3
    
    return 0;
}
```

### Error Handling

```cpp
auto result = decoder.decode(payload);

if (!result) {
    switch (result.error()) {
        case cayene::Error::PayloadEmpty:
            // Handle empty payload
            break;
        case cayene::Error::UnknownDataType:
            // Handle unknown type ID
            break;
        case cayene::Error::BadPayloadFormat:
            // Handle malformed payload
            break;
        case cayene::Error::Unexpected:
            // Handle unexpected error
            break;
    }
}
```

## API Reference

### `cayene::Decoder`

| Method | Description |
|--------|-------------|
| `decode(span<const uint8_t>)` | Decode a Cayene LPP payload to JSON |
| `add_custom_type(id, name, size, decoder)` | Register a custom data type |
| `has_type(id)` | Check if a type ID is registered |
| `remove_custom_type(id)` | Remove a custom type (standard types cannot be removed) |

### `cayene::Error`

| Value | Description |
|-------|-------------|
| `None` | No error |
| `Unexpected` | Unexpected internal error |
| `UnknownDataType` | Unregistered type ID encountered |
| `BadPayloadFormat` | Malformed or incomplete payload |
| `PayloadEmpty` | Empty payload provided |

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CAYENE_BUILD_TESTS` | ON | Build unit tests (requires GoogleTest) |
| `CAYENE_BUILD_EXAMPLES` | ON | Build example programs |
| `CAYENE_ENABLE_WARNINGS` | ON | Enable strict compiler warnings |
| `CAYENE_ENABLE_SANITIZERS` | OFF | Enable AddressSanitizer/UBSan (Debug only) |

### Debug Build with Sanitizers

```bash
cmake --preset debug
cmake --build build/debug
ctest --preset debug
```

## Project Structure

```
cayene_decoder/
├── CMakeLists.txt          # Main CMake configuration
├── CMakePresets.json       # CMake presets for easy building
├── include/cayene/
│   ├── decoder.hpp         # Main decoder class
│   ├── data_type.hpp       # DataType definition
│   └── error.hpp           # Error enumeration
├── src/
│   ├── decoder.cpp         # Decoder implementation
│   └── cayene_v1_defintions.hpp  # Standard type definitions
├── tests/
│   └── decoder_test.cpp    # Unit tests (32 tests)
├── examples/
│   └── basic_example.cpp   # Usage example
├── .clang-format           # Code formatting rules
└── .clang-tidy             # Static analysis configuration
```

## Integration

### As a CMake Subdirectory

```cmake
add_subdirectory(cayene_decoder)
target_link_libraries(your_target PRIVATE cayene::decoder)
```

### Using FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    cayene_decoder
    GIT_REPOSITORY https://github.com/yourusername/cayene_decoder.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(cayene_decoder)
target_link_libraries(your_target PRIVATE cayene::decoder)
```

## License

This project is licensed under the **GNU General Public License v2 (GPLv2)** - see [LICENSE](LICENSE) file for details.

### GPLv2 Considerations

⚠️ **GPLv2 is a copyleft license**:

- Derivative works must also be licensed under GPLv2 or compatible
- Source code must be provided when distributing binaries
- Cannot be used in proprietary/closed-source applications

## Acknowledgments

Project scaffolding (CMake configuration, clang-format, clang-tidy) was generated with AI assistance. The decoder implementation and tests were developed by hand with AI as an auxiliary tool for review.
