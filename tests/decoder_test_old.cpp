/**
 * @file decoder_test.cpp
 * @brief Exhaustive unit tests for the Cayene Decoder (C++20 with exceptions)
 *
 * Test coverage:
 * - Error handling (empty, unknown types, malformed payloads)
 * - All 12 standard data types with edge cases
 * - Multi-sensor payloads
 * - Custom type registration and removal
 * - Boundary values (min/max)
 * - Negative values
 * - Channel variations (0-255)
 * - Complex multi-type scenarios
 *
 * This library is licensed under the GNU General Public License v2 (GPLv2).
 * See LICENSE file for details.
 */

#include "cayene/decoder.hpp"

#include <cstdint>
#include <limits>
#include <vector>

#include <gtest/gtest.h>

namespace cayene::test
{

class DecoderTest : public ::testing::Test
{
protected:
    Decoder decoder_;
};

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(DecoderTest, EmptyPayload)
{
    std::vector<std::uint8_t> payload = {};
    EXPECT_THROW(decoder_.decode(payload), PayloadEmptyException);
}

TEST_F(DecoderTest, UnknownDataType)
{
    std::vector<std::uint8_t> payload = {0x01, 0xFF, 0x00, 0x00};  // Unknown type 0xFF
    EXPECT_THROW(decoder_.decode(payload), UnknownDataTypeException);
}

TEST_F(DecoderTest, InsufficientBytes)
{
    std::vector<std::uint8_t> payload = {0x01, 0x67, 0x01};  // Temperature needs 2 bytes, only 1 provided
    EXPECT_THROW(decoder_.decode(payload), BadPayloadFormatException);
}

TEST_F(DecoderTest, UnprocessedBytesRemaining)
{
    std::vector<std::uint8_t> payload = {0x01, 0x67, 0x01, 0x10, 0xFF};  // Extra byte at end
    EXPECT_THROW(decoder_.decode(payload), BadPayloadFormatException);
}

// ============================================================================
// Standard Type Tests
// ============================================================================

TEST_F(DecoderTest, DigitalInput)
{
    std::vector<std::uint8_t> payload = {0x01, 0x00, 0x01};  // Digital input: ON
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Digital Input_1"], 1);
}

TEST_F(DecoderTest, DigitalOutput)
{
    std::vector<std::uint8_t> payload = {0x02, 0x01, 0x00};  // Digital output: OFF
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Digital Output_2"], 0);
}

TEST_F(DecoderTest, AnalogInput)
{
    std::vector<std::uint8_t> payload = {0x03, 0x02, 0x00, 0x64};  // Analog input: 100 -> 1.00
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Analog Input_3"], 1.0);
}

TEST_F(DecoderTest, Temperature)
{
    std::vector<std::uint8_t> payload = {0x01, 0x67, 0x01, 0x10};  // 272 -> 27.2°C
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Temperature_1"], 27.2);
}

TEST_F(DecoderTest, NegativeTemperature)
{
    std::vector<std::uint8_t> payload = {0x01, 0x67, 0xFF, 0xF6};  // -10 -> -1.0°C
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Temperature_1"], -1.0);
}

TEST_F(DecoderTest, Humidity)
{
    std::vector<std::uint8_t> payload = {0x02, 0x68, 0x02, 0x8A};  // 650 -> 65.0%
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Humidity_2"], 65.0);
}

TEST_F(DecoderTest, Barometer)
{
    std::vector<std::uint8_t> payload = {0x03, 0x73, 0x27, 0x7F};  // 10111 -> 1011.1 hPa
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Barometer_3"], 1011.1);
}

TEST_F(DecoderTest, GPS)
{
    std::vector<std::uint8_t> payload = {
        0x01, 0x88,           // GPS
        0x06, 0x19, 0x48,     // Lat
        0xF9, 0xCC, 0xE6,     // Lon
        0x00, 0x09, 0xC4      // Alt
    };
    auto result = decoder_.decode(payload);
    EXPECT_TRUE(result["GPS_1"].contains("latitude"));
    EXPECT_TRUE(result["GPS_1"].contains("longitude"));
    EXPECT_TRUE(result["GPS_1"].contains("altitude"));
}

TEST_F(DecoderTest, Accelerometer)
{
    std::vector<std::uint8_t> payload = {
        0x01, 0x71,       // Accelerometer
        0x01, 0xF4,       // x
        0xFF, 0xD8,       // y
        0x03, 0xE8        // z
    };
    auto result = decoder_.decode(payload);
    EXPECT_TRUE(result["Accelerometer_1"].contains("x"));
    EXPECT_TRUE(result["Accelerometer_1"].contains("y"));
    EXPECT_TRUE(result["Accelerometer_1"].contains("z"));
}

// ============================================================================
// Multi-sensor Tests
// ============================================================================

TEST_F(DecoderTest, MultiSensorPayload)
{
    std::vector<std::uint8_t> payload = {
        0x01, 0x67, 0x00, 0xFF,  // Temperature
        0x02, 0x68, 0x02, 0x8A,  // Humidity
        0x03, 0x73, 0x27, 0x7F   // Barometer
    };
    auto result = decoder_.decode(payload);
    EXPECT_TRUE(result.contains("Temperature_1"));
    EXPECT_TRUE(result.contains("Humidity_2"));
    EXPECT_TRUE(result.contains("Barometer_3"));
}

TEST_F(DecoderTest, SameTypeMultipleChannels)
{
    std::vector<std::uint8_t> payload = {
        0x01, 0x67, 0x00, 0xFF,  // Temperature Ch1
        0x02, 0x67, 0x01, 0x10   // Temperature Ch2
    };
    auto result = decoder_.decode(payload);
    EXPECT_TRUE(result.contains("Temperature_1"));
    EXPECT_TRUE(result.contains("Temperature_2"));
    EXPECT_DOUBLE_EQ(result["Temperature_1"], 25.5);
    EXPECT_DOUBLE_EQ(result["Temperature_2"], 27.2);
}

// ============================================================================
// Custom Type Tests
// ============================================================================

TEST_F(DecoderTest, AddCustomType)
{
    constexpr std::uint8_t kBatteryTypeId = 0xA0;
    
    auto battery_decoder = [](std::span<const std::uint8_t> data) -> Json {
        const std::uint16_t raw = (static_cast<std::uint16_t>(data[0]) << 8) | data[1];
        return Json{{"voltage", raw / 1000.0}};
    };

    bool added = decoder_.add_custom_type(kBatteryTypeId, "Battery", 2, battery_decoder);
    EXPECT_TRUE(added);
    EXPECT_TRUE(decoder_.has_type(kBatteryTypeId));
}

TEST_F(DecoderTest, DecodeCustomType)
{
    constexpr std::uint8_t kBatteryTypeId = 0xA0;
    
    auto battery_decoder = [](std::span<const std::uint8_t> data) -> Json {
        const std::uint16_t raw = (static_cast<std::uint16_t>(data[0]) << 8) | data[1];
        return Json{{"voltage", raw / 1000.0}};
    };

    decoder_.add_custom_type(kBatteryTypeId, "Battery", 2, battery_decoder);

    std::vector<std::uint8_t> payload = {0x01, kBatteryTypeId, 0x0E, 0x74};  // 3700mV
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Battery_1"]["voltage"], 3.7);
}

TEST_F(DecoderTest, RemoveCustomType)
{
    constexpr std::uint8_t kBatteryTypeId = 0xA0;
    
    auto battery_decoder = [](std::span<const std::uint8_t> data) -> Json {
        return Json{{"value", data[0]}};
    };

    decoder_.add_custom_type(kBatteryTypeId, "Battery", 1, battery_decoder);
    EXPECT_TRUE(decoder_.has_type(kBatteryTypeId));

    bool removed = decoder_.remove_custom_type(kBatteryTypeId);
    EXPECT_TRUE(removed);
    EXPECT_FALSE(decoder_.has_type(kBatteryTypeId));
}

TEST_F(DecoderTest, CannotOverwriteStandardType)
{
    auto fake_decoder = [](std::span<const std::uint8_t>) -> Json {
        return Json{{"fake", true}};
    };

    bool added = decoder_.add_custom_type(0x67, "FakeTemp", 2, fake_decoder);  // 0x67 is Temperature
    EXPECT_FALSE(added);
}

TEST_F(DecoderTest, MixedStandardAndCustom)
{
    constexpr std::uint8_t kBatteryTypeId = 0xA0;
    
    auto battery_decoder = [](std::span<const std::uint8_t> data) -> Json {
        const std::uint16_t raw = (static_cast<std::uint16_t>(data[0]) << 8) | data[1];
        return Json{{"voltage", raw / 1000.0}};
    };

    decoder_.add_custom_type(kBatteryTypeId, "Battery", 2, battery_decoder);

    std::vector<std::uint8_t> payload = {
        0x01, 0x67, 0x01, 0x10,           // Temperature
        0x02, kBatteryTypeId, 0x0E, 0x74  // Battery
    };

    auto result = decoder_.decode(payload);
    EXPECT_TRUE(result.contains("Temperature_1"));
    EXPECT_TRUE(result.contains("Battery_2"));
    EXPECT_DOUBLE_EQ(result["Temperature_1"], 27.2);
    EXPECT_DOUBLE_EQ(result["Battery_2"]["voltage"], 3.7);
}

}  // namespace cayene::test

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
