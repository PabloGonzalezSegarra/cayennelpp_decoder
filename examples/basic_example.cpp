/**
 * @file basic_example.cpp
 * @brief Basic usage examples of the Cayene Decoder library
 *
 * This library is licensed under the GNU General Public License v2 (GPLv2).
 * See LICENSE file for details.
 */

#include <cstdint>
#include <iostream>
#include <vector>

#include "cayene/decoder.hpp"

int main()
{
    using namespace cayene;

    Decoder decoder;

    // =========================================================================
    // Example 1: Simple temperature reading
    // =========================================================================
    std::cout << "=== Example 1: Temperature sensor ===\n\n";

    // Single temperature reading on channel 1: 27.2°C
    // Format: [channel] [type=0x67] [value_high] [value_low]
    std::vector<std::uint8_t> temp_payload = {
        0x01, 0x67, 0x01, 0x10  // Ch1, Temperature: 272 -> 27.2°C
    };

    try
    {
        auto result = decoder.decode(temp_payload);
        std::cout << "Temperature reading:\n";
        std::cout << result.dump(2) << "\n\n";
    }
    catch (const DecoderException& e)
    {
        std::cout << "Error: " << e.what() << "\n\n";
    }

    // =========================================================================
    // Example 2: Multi-sensor payload
    // =========================================================================
    std::cout << "=== Example 2: Multi-sensor payload ===\n\n";

    // Multiple sensor readings in a single payload:
    // - Temperature on channel 1: 25.5°C
    // - Humidity on channel 2: 65.0%
    // - Barometer on channel 3: 1011.1 hPa
    std::vector<std::uint8_t> multi_sensor_payload = {
        0x01, 0x67, 0x00, 0xFF,  // Ch1, Temperature: 255 -> 25.5°C
        0x02, 0x68, 0x02, 0x8A,  // Ch2, Humidity: 650 -> 65.0%
        0x03, 0x73, 0x27, 0x7F   // Ch3, Barometer: 10111 -> 1011.1 hPa
    };

    try
    {
        auto result = decoder.decode(multi_sensor_payload);
        std::cout << "Multi-sensor readings:\n";
        std::cout << result.dump(2) << "\n\n";
    }
    catch (const DecoderException& e)
    {
        std::cout << "Error: " << e.what() << "\n\n";
    }

    // =========================================================================
    // Example 3: GPS coordinates
    // =========================================================================
    std::cout << "=== Example 3: GPS location ===\n\n";

    // GPS payload: Latitude 40.3512°, Longitude -1.4762°, Altitude 640m
    // Format: [channel] [type=0x88] [lat:3bytes] [lon:3bytes] [alt:3bytes]
    std::vector<std::uint8_t> gps_payload = {
        0x01, 0x88,        // Ch1, GPS
        0x06, 0x19, 0x48,  // Latitude: 40.3512°
        0xF9, 0xCC, 0xE6,  // Longitude: -1.4762°
        0x00, 0x09, 0xC4   // Altitude: 2500 -> 25.00m (scaled by 0.01)
    };

    try
    {
        auto result = decoder.decode(gps_payload);
        std::cout << "GPS location:\n";
        std::cout << result.dump(2) << "\n\n";

        const auto& gps = result["GPS_1"];
        std::cout << "  Latitude:  " << gps["latitude"].get<double>() << "°\n";
        std::cout << "  Longitude: " << gps["longitude"].get<double>() << "°\n";
        std::cout << "  Altitude:  " << gps["altitude"].get<double>() << "m\n\n";
    }
    catch (const DecoderException& e)
    {
        std::cout << "Error: " << e.what() << "\n\n";
    }

    // =========================================================================
    // Example 4: Accelerometer
    // =========================================================================
    std::cout << "=== Example 4: Accelerometer ===\n\n";

    // Accelerometer on channel 1: x=0.5g, y=-0.3g, z=1.0g
    // Format: [channel] [type=0x71] [x:2bytes] [y:2bytes] [z:2bytes]
    std::vector<std::uint8_t> accel_payload = {
        0x01, 0x71,  // Ch1, Accelerometer
        0x01, 0xF4,  // x = 500 -> 0.5g
        0xFF, 0xD8,  // y = -40 -> -0.04g (signed)
        0x03, 0xE8   // z = 1000 -> 1.0g
    };

    try
    {
        auto result = decoder.decode(accel_payload);
        std::cout << "Accelerometer reading:\n";
        std::cout << result.dump(2) << "\n\n";
    }
    catch (const DecoderException& e)
    {
        std::cout << "Error: " << e.what() << "\n\n";
    }

    // =========================================================================
    // Example 5: Error handling
    // =========================================================================
    std::cout << "=== Example 5: Error handling ===\n\n";

    // Empty payload (should throw PayloadEmptyException)
    std::vector<std::uint8_t> empty_payload = {};

    try
    {
        auto result = decoder.decode(empty_payload);
        std::cout << "Should not reach here!\n";
    }
    catch (const PayloadEmptyException& e)
    {
        std::cout << "Empty payload error: " << e.what() << "\n\n";
    }
    catch (const DecoderException& e)
    {
        std::cout << "Other error: " << e.what() << "\n\n";
    }

    // Bad payload format (incomplete)
    std::vector<std::uint8_t> bad_payload = {
        0x01, 0x67, 0x01  // Temperature but missing one byte
    };

    try
    {
        auto result = decoder.decode(bad_payload);
        std::cout << "Should not reach here!\n";
    }
    catch (const BadPayloadFormatException& e)
    {
        std::cout << "Bad format error: " << e.what() << "\n\n";
    }
    catch (const DecoderException& e)
    {
        std::cout << "Other error: " << e.what() << "\n\n";
    }

    return 0;
}
