/**
 * @file advanced_example.cpp
 * @brief Advanced usage with custom data types
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

    // Custom type IDs (avoid standard 0x00-0x88 range)
    constexpr std::uint8_t kBatteryVoltage = 0xA0;

    // =========================================================================
    // Example 1: Battery voltage custom type
    // =========================================================================
    std::cout << "=== Example 1: Battery voltage custom type ===\n\n";

    // Register a custom decoder for battery voltage (2 bytes -> mV)
    auto battery_decoder = [](std::span<const std::uint8_t> data) -> Json
    {
        if (data.size() != 2)
        {
            throw BadPayloadFormatException("Battery decoder requires 2 bytes");
        }
        const std::uint16_t raw = (static_cast<std::uint16_t>(data[0]) << 8) | data[1];
        const double voltage = static_cast<double>(raw) / 1000.0;  // mV to V
        return Json{{"voltage", voltage}, {"unit", "V"}};
    };

    bool added = decoder.add_custom_type(kBatteryVoltage, "Battery", 2, battery_decoder);
    std::cout << "Battery voltage type registered: " << (added ? "yes" : "no") << "\n\n";

    // Decode a payload with battery voltage: 3700mV = 3.7V
    std::vector<std::uint8_t> battery_payload = {
        0x01, kBatteryVoltage, 0x0E, 0x74  // Ch1, Battery: 3700mV
    };

    try
    {
        auto result = decoder.decode(battery_payload);
        std::cout << "Decoded battery:\n" << result.dump(2) << "\n\n";
    }
    catch (const DecoderException& e)
    {
        std::cout << "Error: " << e.what() << "\n\n";
    }

    // =========================================================================
    // Example 2: Mixed payload (standard + custom)
    // =========================================================================
    std::cout << "=== Example 2: Mixed payload (standard + custom) ===\n\n";

    // Mixed payload: Temperature + Battery
    std::vector<std::uint8_t> mixed_payload = {
        0x01, 0x67,
        0x01, 0x10,  // Ch1, Temperature: 27.2°C
        0x02, kBatteryVoltage,
        0x0E, 0x74  // Ch2, Battery: 3.7V
    };

    try
    {
        auto result = decoder.decode(mixed_payload);
        std::cout << "Mixed payload decoded:\n" << result.dump(2) << "\n\n";
    }
    catch (const DecoderException& e)
    {
        std::cout << "Error: " << e.what() << "\n\n";
    }

    // =========================================================================
    // Example 3: Managing custom types
    // =========================================================================
    std::cout << "=== Example 3: Managing custom types ===\n\n";

    std::cout << "Has BatteryVoltage type (0xA0): " << decoder.has_type(kBatteryVoltage) << "\n";
    std::cout << "Has Temperature type (0x67): " << decoder.has_type(0x67) << "\n";
    std::cout << "Has unknown type (0xFF): " << decoder.has_type(0xFF) << "\n\n";

    // Remove custom type
    bool removed = decoder.remove_custom_type(kBatteryVoltage);
    std::cout << "Battery type removed: " << (removed ? "yes" : "no") << "\n";
    std::cout << "Has BatteryVoltage after removal: " << decoder.has_type(kBatteryVoltage)
              << "\n\n";

    // Try to decode battery payload after removal (should fail)
    try
    {
        auto result = decoder.decode(battery_payload);
        std::cout << "Should not reach here!\n";
    }
    catch (const UnknownDataTypeException& e)
    {
        std::cout << "Expected error after removal: " << e.what() << "\n\n";
    }
    catch (const DecoderException& e)
    {
        std::cout << "Other error: " << e.what() << "\n\n";
    }

    return 0;
}
