# Cayene Decoder

A modern C++26 library for decoding Cayene LPP (Low Power Payload) format, commonly used in LoRaWAN sensor networks.

## Features

- **Full Cayene LPP v1 Support** — Decode all 12 standard sensor types
- **Custom Type Registration** — Extend with proprietary sensor types  
- **Modern C++26** — Uses `std::expected`, `std::span`, and `std::print`
- **Type-Safe** — Strong typing with explicit error handling
- **Header-Only Friendly** — Single library target, easy integration

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
| 0x88 | GPS | 9 bytes | 0.0001° / 0.01m |

## Requirements

- **Compiler**: Clang 18+ with C++26 support
- **Build System**: CMake 3.25+
- **Build Tool**: Ninja (recommended)

## Quick Start

```bash
# Configure
cmake --preset release

# Build  
cmake --build build/release

# Test
ctest --test-dir build/release --output-on-failure

# Run example
./build/release/examples/basic_example
```

## Usage

### Basic Decoding

```cpp
#include <cayene/decoder.hpp>
#include <print>

int main() {
    cayene::Decoder decoder;
    
    // Temperature (27.2°C) + Humidity (60.0%)
    std::vector<std::uint8_t> payload = {
        0x01, 0x67, 0x01, 0x10,  // Ch1, Temperature
        0x02, 0x68, 0x02, 0x58   // Ch2, Humidity
    };
    
    auto result = decoder.decode(payload);
    
    if (result) {
        std::println("{}", result.value().dump(2));
        // {"Humidity_2": 60.0, "Temperature_1": 27.2}
    }
}
```

### Custom Types

```cpp
// Register a battery voltage sensor (2 bytes, millivolts)
decoder.add_custom_type(
    0xFE, "Battery", 2,
    [](std::span<const std::uint8_t> data) -> cayene::Json {
        auto mv = (static_cast<unsigned>(data[0]) << 8) | data[1];
        return cayene::Json(mv / 1000.0);  // Convert to volts
    }
);
```

### Error Handling

```cpp
auto result = decoder.decode(payload);

if (!result) {
    switch (result.error()) {
        case cayene::Error::PayloadEmpty:
            // Empty input
            break;
        case cayene::Error::UnknownDataType:
            // Unregistered type ID
            break;
        case cayene::Error::BadPayloadFormat:
            // Malformed or truncated payload
            break;
    }
}
```

## API

### `cayene::Decoder`

| Method | Description |
|--------|-------------|
| `decode(span<const uint8_t>)` | Decode payload → `expected<Json, Error>` |
| `add_custom_type(id, name, size, fn)` | Register custom type → `bool` |
| `has_type(id)` | Check if type exists → `bool` |
| `remove_custom_type(id)` | Remove custom type → `bool` |

### `cayene::Error`

| Value | Description |
|-------|-------------|
| `PayloadEmpty` | Empty payload provided |
| `UnknownDataType` | Unregistered type ID encountered |
| `BadPayloadFormat` | Incomplete or malformed payload |
| `Unexpected` | Internal error |

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CAYENE_BUILD_TESTS` | ON | Build unit tests |
| `CAYENE_BUILD_EXAMPLES` | ON | Build examples |
| `CAYENE_ENABLE_SANITIZERS` | OFF | Enable ASan/UBSan (Debug) |

## Project Structure

```
cayene_decoder/
├── include/cayene/
│   ├── decoder.hpp      # Main API
│   ├── data_type.hpp    # DataType class
│   └── error.hpp        # Error enum
├── src/
│   └── decoder.cpp      # Implementation
├── tests/
│   └── decoder_test.cpp # 77 unit tests
└── examples/
    ├── basic_example.cpp
    └── advanced_example.cpp
```

## Integration

### CMake Subdirectory

```cmake
add_subdirectory(cayene_decoder)
target_link_libraries(your_target PRIVATE cayene::decoder)
```

### FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(cayene_decoder
    GIT_REPOSITORY https://github.com/user/cayene_decoder.git
    GIT_TAG main
)
FetchContent_MakeAvailable(cayene_decoder)
target_link_libraries(your_target PRIVATE cayene::decoder)
```

## License

GNU General Public License v2 (GPLv2). See [LICENSE](LICENSE).

**Note**: GPLv2 is copyleft — derivative works must also be GPLv2.
