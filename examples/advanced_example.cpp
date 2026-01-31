/**
 * @file advanced_example.cpp
 * @brief Advanced example demonstrating custom Cayene LPP types
 *
 * This example shows:
 * - How to register custom data types for proprietary sensors
 * - How to decode complex multi-byte custom types
 * - How to mix custom and standard types in payloads
 * - How to manage custom types (add, check, remove)
 *
 * For basic usage, see basic_example.cpp
 *
 * This library is licensed under the GNU General Public License v2 (GPLv2).
 */

#include <cstdint>
#include <cstdlib>
#include <print>
#include <span>
#include <vector>

#include "cayene/decoder.hpp"

// Custom type IDs (use values not used by standard Cayene LPP)
// Standard types use: 0x00-0x03, 0x65-0x68, 0x71, 0x73, 0x86, 0x88
namespace custom_types
{
constexpr std::uint8_t kBatteryVoltage = 0xF0;    // 2 bytes: millivolts
constexpr std::uint8_t kRgbColor = 0xF1;          // 3 bytes: R, G, B
constexpr std::uint8_t kDeviceStatus = 0xF2;      // 1 byte: status flags
constexpr std::uint8_t kPowerConsumption = 0xF3;  // 4 bytes: milliwatts (int32)
constexpr std::uint8_t kUuid = 0xF4;              // 16 bytes: UUID
}  // namespace custom_types

int main()
{
    using namespace cayene;

    Decoder decoder;

    // =========================================================================
    // Example 1: Register a simple custom type (Battery Voltage)
    // =========================================================================
    std::println("=== Example 1: Battery voltage custom type ===\n");

    // Battery voltage: 2 bytes representing millivolts, decoded to volts
    bool added = decoder.add_custom_type(
        custom_types::kBatteryVoltage, "BatteryVoltage",
        2,  // 2 bytes of data
        [](std::span<const std::uint8_t> data) -> Json
        {
            // Big-endian 16-bit unsigned value
            auto millivolts = static_cast<std::uint16_t>((static_cast<unsigned>(data[0]) << 8U) |
                                                         static_cast<unsigned>(data[1]));
            // Return as volts
            return Json(millivolts / 1000.0);
        });

    std::println("Battery voltage type registered: {}", added);

    // Test decoding
    std::vector<std::uint8_t> battery_payload = {
        0x01, custom_types::kBatteryVoltage, 0x0C, 0xE4  // 3300mV -> 3.3V
    };

    auto result = decoder.decode(battery_payload);
    if (result)
    {
        std::println("Decoded battery: {}\n", result.value().dump(2));
    }

    // =========================================================================
    // Example 2: Custom type returning a JSON object (RGB Color)
    // =========================================================================
    std::println("=== Example 2: RGB color custom type ===\n");

    // RGB Color: 3 bytes representing R, G, B values (0-255 each)
    decoder.add_custom_type(custom_types::kRgbColor, "RGBColor", 3,
                            [](std::span<const std::uint8_t> data) -> Json
                            {
                                Json color = Json::object();
                                color["red"] = static_cast<int>(data[0]);
                                color["green"] = static_cast<int>(data[1]);
                                color["blue"] = static_cast<int>(data[2]);
                                // Also include hex representation
                                char hex[8];
                                std::snprintf(hex, sizeof(hex), "#%02X%02X%02X", data[0], data[1],
                                              data[2]);
                                color["hex"] = hex;
                                return color;
                            });

    std::vector<std::uint8_t> rgb_payload = {
        0x01, custom_types::kRgbColor, 0xFF, 0x80, 0x00  // Orange: RGB(255, 128, 0)
    };

    result = decoder.decode(rgb_payload);
    if (result)
    {
        std::println("Decoded RGB color: {}\n", result.value().dump(2));
    }

    // =========================================================================
    // Example 3: Custom type with bit flags (Device Status)
    // =========================================================================
    std::println("=== Example 3: Device status flags ===\n");

    // Device status: 1 byte with bit flags
    // Bit 0: Power OK
    // Bit 1: Sensor OK
    // Bit 2: Network connected
    // Bit 3: Low battery warning
    // Bit 4-7: Reserved
    decoder.add_custom_type(custom_types::kDeviceStatus, "DeviceStatus", 1,
                            [](std::span<const std::uint8_t> data) -> Json
                            {
                                std::uint8_t status = data[0];
                                Json flags = Json::object();
                                flags["power_ok"] = (status & 0x01U) != 0;
                                flags["sensor_ok"] = (status & 0x02U) != 0;
                                flags["network_connected"] = (status & 0x04U) != 0;
                                flags["low_battery"] = (status & 0x08U) != 0;
                                flags["raw_value"] = static_cast<int>(status);
                                return flags;
                            });

    std::vector<std::uint8_t> status_payload = {
        0x01, custom_types::kDeviceStatus, 0x0F  // All flags set
    };

    result = decoder.decode(status_payload);
    if (result)
    {
        std::println("Device status: {}\n", result.value().dump(2));
    }

    // =========================================================================
    // Example 4: Signed 32-bit custom type (Power Consumption)
    // =========================================================================
    std::println("=== Example 4: Power consumption (signed 32-bit) ===\n");

    decoder.add_custom_type(
        custom_types::kPowerConsumption, "PowerConsumption_mW", 4,
        [](std::span<const std::uint8_t> data) -> Json
        {
            // Big-endian signed 32-bit value
            auto value = static_cast<std::int32_t>((static_cast<std::uint32_t>(data[0]) << 24U) |
                                                   (static_cast<std::uint32_t>(data[1]) << 16U) |
                                                   (static_cast<std::uint32_t>(data[2]) << 8U) |
                                                   static_cast<std::uint32_t>(data[3]));
            return Json(value);
        });

    std::vector<std::uint8_t> power_payload = {
        0x01, custom_types::kPowerConsumption, 0x00, 0x00, 0x09, 0xC4  // 2500 mW
    };

    result = decoder.decode(power_payload);
    if (result)
    {
        std::println("Power consumption: {}\n", result.value().dump(2));
    }

    // =========================================================================
    // Example 5: Large custom type (UUID - 16 bytes)
    // =========================================================================
    std::println("=== Example 5: UUID (16 bytes) ===\n");

    decoder.add_custom_type(
        custom_types::kUuid, "DeviceUUID", 16,
        [](std::span<const std::uint8_t> data) -> Json
        {
            // Format as standard UUID string: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
            char uuid_str[37];
            std::snprintf(uuid_str, sizeof(uuid_str),
                          "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                          data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
                          data[8], data[9], data[10], data[11], data[12], data[13], data[14],
                          data[15]);
            return Json(uuid_str);
        });

    std::vector<std::uint8_t> uuid_payload = {
        0x01, custom_types::kUuid,
        0x12, 0x34,
        0x56, 0x78,  // First 4 bytes
        0x9A, 0xBC,  // Bytes 5-6
        0xDE, 0xF0,  // Bytes 7-8
        0x11, 0x22,  // Bytes 9-10
        0x33, 0x44,
        0x55, 0x66,
        0x77, 0x88  // Last 6 bytes
    };

    result = decoder.decode(uuid_payload);
    if (result)
    {
        std::println("Device UUID: {}\n", result.value().dump(2));
    }

    // =========================================================================
    // Example 6: Mix custom and standard types
    // =========================================================================
    std::println("=== Example 6: Mixed payload (standard + custom) ===\n");

    std::vector<std::uint8_t> mixed_payload = {
        // Standard temperature sensor
        0x01, 0x67, 0x01, 0x10,  // Ch1, Temperature: 27.2°C

        // Custom battery voltage
        0x02, custom_types::kBatteryVoltage, 0x0D, 0x48,  // Ch2, 3400mV -> 3.4V

        // Standard humidity sensor
        0x03, 0x68, 0x02, 0x8A,  // Ch3, Humidity: 65.0%

        // Custom device status
        0x04, custom_types::kDeviceStatus, 0x07,  // Ch4, Power+Sensor+Network OK

        // Custom RGB LED status
        0x05, custom_types::kRgbColor, 0x00, 0xFF, 0x00  // Ch5, Green LED
    };

    result = decoder.decode(mixed_payload);
    if (result)
    {
        std::println("Mixed payload decoded:");
        std::println("{}\n", result.value().dump(2));
    }

    // =========================================================================
    // Example 7: Manage custom types
    // =========================================================================
    std::println("=== Example 7: Managing custom types ===\n");

    // Check if types exist
    std::println("Has BatteryVoltage type (0x{:02X}): {}", custom_types::kBatteryVoltage,
                 decoder.has_type(custom_types::kBatteryVoltage));

    std::println("Has Temperature type (0x67): {}", decoder.has_type(0x67));

    std::println("Has unknown type (0xFF): {}", decoder.has_type(0xFF));

    // Try to add duplicate type (should fail)
    bool duplicate_added =
        decoder.add_custom_type(custom_types::kBatteryVoltage, "Duplicate", 2,
                                [](std::span<const std::uint8_t>) { return Json(0); });
    std::println("\nTried to add duplicate type: {}",
                 duplicate_added ? "succeeded" : "failed (expected)");

    // Try to overwrite standard type (should fail)
    bool overwrite_standard = decoder.add_custom_type(
        0x67, "FakeTemp", 2, [](std::span<const std::uint8_t>) { return Json(0); });
    std::println("Tried to overwrite standard type: {}",
                 overwrite_standard ? "succeeded" : "failed (expected)");

    // Remove a custom type
    bool removed = decoder.remove_custom_type(custom_types::kBatteryVoltage);
    std::println("\nRemoved BatteryVoltage type: {}", removed);
    std::println("Has BatteryVoltage type after removal: {}",
                 decoder.has_type(custom_types::kBatteryVoltage));

    // Decoding with removed type fails
    result = decoder.decode(battery_payload);
    if (!result)
    {
        std::println("Decoding after removal: error {} (UnknownDataType)",
                     static_cast<int>(result.error()));
    }

    // Try to remove standard type (should fail)
    bool remove_standard = decoder.remove_custom_type(0x67);
    std::println("\nTried to remove standard type: {}",
                 remove_standard ? "succeeded" : "failed (expected)");

    std::println("\n=== Advanced example completed ===");

    return EXIT_SUCCESS;
}
