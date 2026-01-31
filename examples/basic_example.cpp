/**
 * @file basic_example.cpp
 * @brief Basic example demonstrating Cayene LPP decoding
 *
 * This example shows:
 * - How to decode standard Cayene LPP sensor payloads
 * - How to handle common sensor types (temperature, humidity, GPS, accelerometer)
 * - Basic error handling
 *
 * For advanced usage with custom types, see advanced_example.cpp
 *
 * This library is licensed under the GNU General Public License v2 (GPLv2).
 */

#include <cstdint>
#include <cstdlib>
#include <print>
#include <vector>

#include "cayene/decoder.hpp"

int main()
{
    using namespace cayene;

    Decoder decoder;

    // =========================================================================
    // Example 1: Simple temperature reading
    // =========================================================================
    std::println("=== Example 1: Temperature sensor ===\n");

    // Single temperature reading on channel 1: 27.2°C
    // Format: [channel] [type=0x67] [value_high] [value_low]
    std::vector<std::uint8_t> temp_payload = {
        0x01, 0x67, 0x01, 0x10  // Ch1, Temperature: 272 -> 27.2°C
    };

    auto result = decoder.decode(temp_payload);

    if (result)
    {
        std::println("Temperature reading:");
        std::println("{}\n", result.value().dump(2));
    }
    else
    {
        std::println("Error: {}\n", static_cast<int>(result.error()));
    }

    // =========================================================================
    // Example 2: Multi-sensor payload
    // =========================================================================
    std::println("=== Example 2: Multi-sensor payload ===\n");

    // Multiple sensor readings in a single payload:
    // - Temperature on channel 1: 25.5°C
    // - Humidity on channel 2: 65.0%
    // - Barometer on channel 3: 1013.5 hPa
    std::vector<std::uint8_t> multi_sensor_payload = {
        0x01, 0x67, 0x00, 0xFF,  // Ch1, Temperature: 255 -> 25.5°C
        0x02, 0x68, 0x02, 0x8A,  // Ch2, Humidity: 650 -> 65.0%
        0x03, 0x73, 0x27, 0x7F   // Ch3, Barometer: 10111 -> 1011.1 hPa
    };

    result = decoder.decode(multi_sensor_payload);

    if (result)
    {
        std::println("Multi-sensor readings:");
        std::println("{}\n", result.value().dump(2));
    }

    // =========================================================================
    // Example 3: GPS coordinates
    // =========================================================================
    std::println("=== Example 3: GPS location ===\n");

    // GPS payload: Latitude 40.3512°, Longitude -1.4762°, Altitude 640m
    // Format: [channel] [type=0x88] [lat:3bytes] [lon:3bytes] [alt:3bytes]
    std::vector<std::uint8_t> gps_payload = {
        0x01, 0x88,        // Ch1, GPS type
        0x06, 0x28, 0x38,  // Latitude: 403512 -> 40.3512°
        0xFF, 0xC6, 0x56,  // Longitude: -14762 -> -1.4762°
        0x00, 0xFA, 0x00   // Altitude: 64000 -> 640.00m
    };

    result = decoder.decode(gps_payload);

    if (result)
    {
        std::println("GPS location:");
        std::println("{}\n", result.value().dump(2));

        // Access individual fields
        const auto& gps = result.value()["GPS_1"];
        std::println("  Latitude:  {:.4f}°", gps["latitude"].get<double>());
        std::println("  Longitude: {:.4f}°", gps["longitude"].get<double>());
        std::println("  Altitude:  {:.2f}m\n", gps["altitude"].get<double>());
    }

    // =========================================================================
    // Example 4: Accelerometer data
    // =========================================================================
    std::println("=== Example 4: Accelerometer ===\n");

    // Accelerometer reading: x=0.5G, y=-0.3G, z=1.0G
    // Format: [channel] [type=0x71] [x:2bytes] [y:2bytes] [z:2bytes]
    std::vector<std::uint8_t> accel_payload = {
        0x01, 0x71,  // Ch1, Accelerometer type
        0x01, 0xF4,  // X: 500 -> 0.500 G
        0xFE, 0xD4,  // Y: -300 -> -0.300 G
        0x03, 0xE8   // Z: 1000 -> 1.000 G
    };

    result = decoder.decode(accel_payload);

    if (result)
    {
        std::println("Accelerometer reading:");
        std::println("{}\n", result.value().dump(2));
    }

    // =========================================================================
    // Example 5: Error handling
    // =========================================================================
    std::println("=== Example 5: Error handling ===\n");

    // Empty payload
    std::vector<std::uint8_t> empty_payload;
    result = decoder.decode(empty_payload);
    if (!result)
    {
        std::println("Empty payload error: {} (PayloadEmpty)", static_cast<int>(result.error()));
    }

    // Unknown type (0xFF is not a standard Cayene LPP type)
    std::vector<std::uint8_t> unknown_type_payload = {0x01, 0xFF, 0x00};
    result = decoder.decode(unknown_type_payload);
    if (!result)
    {
        std::println("Unknown type error: {} (UnknownDataType)", static_cast<int>(result.error()));
    }

    // Incomplete payload (temperature needs 2 data bytes, only 1 provided)
    std::vector<std::uint8_t> incomplete_payload = {0x01, 0x67, 0x00};
    result = decoder.decode(incomplete_payload);
    if (!result)
    {
        std::println("Incomplete payload error: {} (BadPayloadFormat)",
                     static_cast<int>(result.error()));
    }

    std::println("\n=== Basic example completed ===");

    return EXIT_SUCCESS;
}
