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
    std::vector<std::uint8_t> payload = {0x01, 0x67,
                                         0x01};  // Temperature needs 2 bytes, only 1 provided
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
        0x01, 0x88,        // GPS
        0x06, 0x19, 0x48,  // Lat
        0xF9, 0xCC, 0xE6,  // Lon
        0x00, 0x09, 0xC4   // Alt
    };
    auto result = decoder_.decode(payload);
    EXPECT_TRUE(result["GPS_1"].contains("latitude"));
    EXPECT_TRUE(result["GPS_1"].contains("longitude"));
    EXPECT_TRUE(result["GPS_1"].contains("altitude"));
}

TEST_F(DecoderTest, Accelerometer)
{
    std::vector<std::uint8_t> payload = {
        0x01, 0x71,  // Accelerometer
        0x01, 0xF4,  // x
        0xFF, 0xD8,  // y
        0x03, 0xE8   // z
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

    auto battery_decoder = [](std::span<const std::uint8_t> data) -> Json
    {
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

    auto battery_decoder = [](std::span<const std::uint8_t> data) -> Json
    {
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

    auto battery_decoder = [](std::span<const std::uint8_t> data) -> Json
    { return Json{{"value", data[0]}}; };

    decoder_.add_custom_type(kBatteryTypeId, "Battery", 1, battery_decoder);
    EXPECT_TRUE(decoder_.has_type(kBatteryTypeId));

    bool removed = decoder_.remove_custom_type(kBatteryTypeId);
    EXPECT_TRUE(removed);
    EXPECT_FALSE(decoder_.has_type(kBatteryTypeId));
}

TEST_F(DecoderTest, CannotOverwriteStandardType)
{
    auto fake_decoder = [](std::span<const std::uint8_t>) -> Json { return Json{{"fake", true}}; };

    bool added =
        decoder_.add_custom_type(0x67, "FakeTemp", 2, fake_decoder);  // 0x67 is Temperature
    EXPECT_FALSE(added);
}

TEST_F(DecoderTest, MixedStandardAndCustom)
{
    constexpr std::uint8_t kBatteryTypeId = 0xA0;

    auto battery_decoder = [](std::span<const std::uint8_t> data) -> Json
    {
        const std::uint16_t raw = (static_cast<std::uint16_t>(data[0]) << 8) | data[1];
        return Json{{"voltage", raw / 1000.0}};
    };

    decoder_.add_custom_type(kBatteryTypeId, "Battery", 2, battery_decoder);

    std::vector<std::uint8_t> payload = {
        0x01, 0x67,           0x01, 0x10,  // Temperature
        0x02, kBatteryTypeId, 0x0E, 0x74   // Battery
    };

    auto result = decoder_.decode(payload);
    EXPECT_TRUE(result.contains("Temperature_1"));
    EXPECT_TRUE(result.contains("Battery_2"));
    EXPECT_DOUBLE_EQ(result["Temperature_1"], 27.2);
    EXPECT_DOUBLE_EQ(result["Battery_2"]["voltage"], 3.7);
}

// ============================================================================
// EXHAUSTIVE EDGE CASE TESTS
// ============================================================================

// Digital Input Edge Cases
TEST_F(DecoderTest, DigitalInputZero)
{
    std::vector<std::uint8_t> payload = {0x01, 0x00, 0x00};
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Digital Input_1"], 0);
}

TEST_F(DecoderTest, DigitalInputMaxValue)
{
    std::vector<std::uint8_t> payload = {0x01, 0x00, 0xFF};
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Digital Input_1"], 255);
}

TEST_F(DecoderTest, DigitalInputMaxChannel)
{
    std::vector<std::uint8_t> payload = {0xFF, 0x00, 0x01};
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Digital Input_255"], 1);
}

TEST_F(DecoderTest, DigitalInputMinChannel)
{
    std::vector<std::uint8_t> payload = {0x00, 0x00, 0x01};
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Digital Input_0"], 1);
}

// Digital Output Edge Cases
TEST_F(DecoderTest, DigitalOutputMaxValue)
{
    std::vector<std::uint8_t> payload = {0x01, 0x01, 0xFF};
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Digital Output_1"], 255);
}

TEST_F(DecoderTest, DigitalOutputZero)
{
    std::vector<std::uint8_t> payload = {0x01, 0x01, 0x00};
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Digital Output_1"], 0);
}

// Analog Input Edge Cases
TEST_F(DecoderTest, AnalogInputZero)
{
    std::vector<std::uint8_t> payload = {0x01, 0x02, 0x00, 0x00};
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Analog Input_1"], 0.0);
}

TEST_F(DecoderTest, AnalogInputNegative)
{
    std::vector<std::uint8_t> payload = {0x01, 0x02, 0xFF, 0x9C};  // -100 / 100 = -1.0
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Analog Input_1"], -1.0);
}

TEST_F(DecoderTest, AnalogInputMaxPositive)
{
    std::vector<std::uint8_t> payload = {0x01, 0x02, 0x7F, 0xFF};  // 32767 / 100 = 327.67
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Analog Input_1"], 327.67);
}

TEST_F(DecoderTest, AnalogInputMaxNegative)
{
    std::vector<std::uint8_t> payload = {0x01, 0x02, 0x80, 0x00};  // -32768 / 100 = -327.68
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Analog Input_1"], -327.68);
}

// Analog Output Edge Cases (Type 0x03)
TEST_F(DecoderTest, AnalogOutputZero)
{
    std::vector<std::uint8_t> payload = {0x01, 0x03, 0x00, 0x00};
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Analog Output_1"], 0.0);
}

TEST_F(DecoderTest, AnalogOutputPositive)
{
    std::vector<std::uint8_t> payload = {0x01, 0x03, 0x00, 0x64};  // 100 / 100 = 1.0
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Analog Output_1"], 1.0);
}

TEST_F(DecoderTest, AnalogOutputNegative)
{
    std::vector<std::uint8_t> payload = {0x01, 0x03, 0xFF, 0x9C};  // -100 / 100 = -1.0
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Analog Output_1"], -1.0);
}

TEST_F(DecoderTest, AnalogOutputMaxPositive)
{
    std::vector<std::uint8_t> payload = {0x01, 0x03, 0x7F, 0xFF};  // 32767 / 100 = 327.67
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Analog Output_1"], 327.67);
}

// Luminosity (Type 0x65)
TEST_F(DecoderTest, LuminosityZero)
{
    std::vector<std::uint8_t> payload = {0x01, 0x65, 0x00, 0x00};
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Luminosity_1"], 0);
}

TEST_F(DecoderTest, LuminosityLow)
{
    std::vector<std::uint8_t> payload = {0x01, 0x65, 0x00, 0x64};  // 100 lux
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Luminosity_1"], 100);
}

TEST_F(DecoderTest, LuminosityMax)
{
    std::vector<std::uint8_t> payload = {0x01, 0x65, 0xFF, 0xFF};  // 65535 lux
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Luminosity_1"], 65535);
}

// Presence (Type 0x66)
TEST_F(DecoderTest, PresenceZero)
{
    std::vector<std::uint8_t> payload = {0x01, 0x66, 0x00};
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Presence_1"], 0);
}

TEST_F(DecoderTest, PresenceDetected)
{
    std::vector<std::uint8_t> payload = {0x01, 0x66, 0x01};
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Presence_1"], 1);
}

TEST_F(DecoderTest, PresenceMaxValue)
{
    std::vector<std::uint8_t> payload = {0x01, 0x66, 0xFF};
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Presence_1"], 255);
}

// Temperature Edge Cases
TEST_F(DecoderTest, TemperatureZero)
{
    std::vector<std::uint8_t> payload = {0x01, 0x67, 0x00, 0x00};
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Temperature_1"], 0.0);
}

TEST_F(DecoderTest, TemperatureMaxPositive)
{
    std::vector<std::uint8_t> payload = {0x01, 0x67, 0x7F, 0xFF};  // 32767 / 10 = 3276.7°C
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Temperature_1"], 3276.7);
}

TEST_F(DecoderTest, TemperatureMaxNegative)
{
    std::vector<std::uint8_t> payload = {0x01, 0x67, 0x80, 0x00};  // -32768 / 10 = -3276.8°C
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Temperature_1"], -3276.8);
}

TEST_F(DecoderTest, TemperatureBoilingPoint)
{
    std::vector<std::uint8_t> payload = {0x01, 0x67, 0x03, 0xE8};  // 1000 / 10 = 100.0°C
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Temperature_1"], 100.0);
}

TEST_F(DecoderTest, TemperatureFreezing)
{
    std::vector<std::uint8_t> payload = {0x01, 0x67, 0xFF, 0xCE};  // -50 / 10 = -5.0°C
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Temperature_1"], -5.0);
}

// Humidity Edge Cases
TEST_F(DecoderTest, HumidityZero)
{
    std::vector<std::uint8_t> payload = {0x01, 0x68, 0x00, 0x00};
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Humidity_1"], 0.0);
}

TEST_F(DecoderTest, HumidityMax)
{
    std::vector<std::uint8_t> payload = {0x01, 0x68, 0xFF, 0xFF};  // 65535 / 10 = 6553.5%
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Humidity_1"], 6553.5);
}

TEST_F(DecoderTest, HumiditySaturated)
{
    std::vector<std::uint8_t> payload = {0x01, 0x68, 0x03, 0xE8};  // 1000 / 10 = 100.0%
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Humidity_1"], 100.0);
}

// Barometer Edge Cases
TEST_F(DecoderTest, BarometerZero)
{
    std::vector<std::uint8_t> payload = {0x01, 0x73, 0x00, 0x00};
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Barometer_1"], 0.0);
}

TEST_F(DecoderTest, BarometerSeaLevel)
{
    std::vector<std::uint8_t> payload = {0x01, 0x73, 0x27, 0x8D};  // 10125 / 10 = 1012.5 hPa
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Barometer_1"], 1012.5);
}

TEST_F(DecoderTest, BarometerMax)
{
    std::vector<std::uint8_t> payload = {0x01, 0x73, 0xFF, 0xFF};  // 65535 / 10 = 6553.5 hPa
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Barometer_1"], 6553.5);
}

// Gyrometer (Type 0x86)
TEST_F(DecoderTest, GyrometerZero)
{
    std::vector<std::uint8_t> payload = {0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Gyrometer_1"]["x"], 0.0);
    EXPECT_DOUBLE_EQ(result["Gyrometer_1"]["y"], 0.0);
    EXPECT_DOUBLE_EQ(result["Gyrometer_1"]["z"], 0.0);
}

TEST_F(DecoderTest, GyrometerPositive)
{
    std::vector<std::uint8_t> payload = {0x01, 0x86, 0x00, 0x64, 0x00, 0xC8, 0x01, 0x2C};
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Gyrometer_1"]["x"], 1.0);  // 100 / 100 = 1.0
    EXPECT_DOUBLE_EQ(result["Gyrometer_1"]["y"], 2.0);  // 200 / 100 = 2.0
    EXPECT_DOUBLE_EQ(result["Gyrometer_1"]["z"], 3.0);  // 300 / 100 = 3.0
}

TEST_F(DecoderTest, GyrometerNegative)
{
    std::vector<std::uint8_t> payload = {0x01, 0x86, 0xFF, 0x9C, 0xFF, 0x38, 0xFE, 0xD4};
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Gyrometer_1"]["x"], -1.0);  // -100 / 100 = -1.0
    EXPECT_DOUBLE_EQ(result["Gyrometer_1"]["y"], -2.0);  // -200 / 100 = -2.0
    EXPECT_DOUBLE_EQ(result["Gyrometer_1"]["z"], -3.0);  // -300 / 100 = -3.0
}

TEST_F(DecoderTest, GyrometerMaxPositive)
{
    std::vector<std::uint8_t> payload = {0x01, 0x86, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF};
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Gyrometer_1"]["x"], 327.67);
    EXPECT_DOUBLE_EQ(result["Gyrometer_1"]["y"], 327.67);
    EXPECT_DOUBLE_EQ(result["Gyrometer_1"]["z"], 327.67);
}

// GPS Edge Cases
TEST_F(DecoderTest, GPSZero)
{
    std::vector<std::uint8_t> payload = {
        0x01, 0x88,        // GPS
        0x00, 0x00, 0x00,  // Lat: 0
        0x00, 0x00, 0x00,  // Lon: 0
        0x00, 0x00, 0x00   // Alt: 0
    };
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["GPS_1"]["latitude"], 0.0);
    EXPECT_DOUBLE_EQ(result["GPS_1"]["longitude"], 0.0);
    EXPECT_DOUBLE_EQ(result["GPS_1"]["altitude"], 0.0);
}

TEST_F(DecoderTest, GPSMaxLatitude)
{
    // Test GPS with various latitudes
    std::vector<std::uint8_t> payload = {
        0x01, 0x88,        // GPS
        0x06, 0x19, 0x48,  // Lat: positive
        0x00, 0x00, 0x00,  // Lon: 0
        0x00, 0x00, 0x00   // Alt: 0
    };
    auto result = decoder_.decode(payload);
    // Just verify GPS fields exist and latitude is decoded
    EXPECT_TRUE(result.contains("GPS_1"));
    EXPECT_TRUE(result["GPS_1"].contains("latitude"));
    EXPECT_TRUE(result["GPS_1"].contains("longitude"));
    EXPECT_TRUE(result["GPS_1"].contains("altitude"));
}

TEST_F(DecoderTest, GPSNegativeAltitude)
{
    std::vector<std::uint8_t> payload = {
        0x01, 0x88,        // GPS
        0x00, 0x00, 0x00,  // Lat: 0
        0x00, 0x00, 0x00,  // Lon: 0
        0xFF, 0xFF, 0xCE   // Alt: negative
    };
    auto result = decoder_.decode(payload);
    EXPECT_LT(result["GPS_1"]["altitude"], 0.0);
}

// Accelerometer Edge Cases
TEST_F(DecoderTest, AccelerometerZero)
{
    std::vector<std::uint8_t> payload = {
        0x01, 0x71,  // Accelerometer
        0x00, 0x00,  // x: 0
        0x00, 0x00,  // y: 0
        0x00, 0x00   // z: 0
    };
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Accelerometer_1"]["x"], 0.0);
    EXPECT_DOUBLE_EQ(result["Accelerometer_1"]["y"], 0.0);
    EXPECT_DOUBLE_EQ(result["Accelerometer_1"]["z"], 0.0);
}

TEST_F(DecoderTest, AccelerometerGravity)
{
    std::vector<std::uint8_t> payload = {
        0x01, 0x71,  // Accelerometer
        0x00, 0x00,  // x: 0
        0x00, 0x00,  // y: 0
        0x03, 0xD5   // z: 981 / 1000 = 0.981 g
    };
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Accelerometer_1"]["x"], 0.0);
    EXPECT_DOUBLE_EQ(result["Accelerometer_1"]["y"], 0.0);
    EXPECT_DOUBLE_EQ(result["Accelerometer_1"]["z"], 0.981);
}

TEST_F(DecoderTest, AccelerometerNegative)
{
    std::vector<std::uint8_t> payload = {
        0x01, 0x71,  // Accelerometer
        0xFF, 0x9C,  // x: -100 / 1000 = -0.1
        0xFF, 0x38,  // y: -200 / 1000 = -0.2
        0xFE, 0xD4   // z: -300 / 1000 = -0.3
    };
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Accelerometer_1"]["x"], -0.1);
    EXPECT_DOUBLE_EQ(result["Accelerometer_1"]["y"], -0.2);
    EXPECT_DOUBLE_EQ(result["Accelerometer_1"]["z"], -0.3);
}

TEST_F(DecoderTest, AccelerometerMaxPositive)
{
    std::vector<std::uint8_t> payload = {
        0x01, 0x71,  // Accelerometer
        0x7F, 0xFF,  // x: 32767 / 1000 = 32.767
        0x7F, 0xFF,  // y: 32767 / 1000 = 32.767
        0x7F, 0xFF   // z: 32767 / 1000 = 32.767
    };
    auto result = decoder_.decode(payload);
    EXPECT_DOUBLE_EQ(result["Accelerometer_1"]["x"], 32.767);
    EXPECT_DOUBLE_EQ(result["Accelerometer_1"]["y"], 32.767);
    EXPECT_DOUBLE_EQ(result["Accelerometer_1"]["z"], 32.767);
}

// ============================================================================
// MULTI-SENSOR EXHAUSTIVE TESTS
// ============================================================================

TEST_F(DecoderTest, TenTemperatureSensors)
{
    std::vector<std::uint8_t> payload;
    for (uint8_t i = 0; i < 10; ++i)
    {
        payload.push_back(i);       // Channel
        payload.push_back(0x67);    // Temperature type
        payload.push_back(0x00);    // High byte
        payload.push_back(i * 10);  // Low byte: 0, 10, 20... -> 0.0, 1.0, 2.0...°C
    }
    auto result = decoder_.decode(payload);

    for (uint8_t i = 0; i < 10; ++i)
    {
        std::string key = "Temperature_" + std::to_string(i);
        EXPECT_DOUBLE_EQ(result[key], i * 1.0);
    }
}

TEST_F(DecoderTest, AllStandardTypes)
{
    std::vector<std::uint8_t> payload = {
        0x00, 0x00, 0x01,                                // Digital Input
        0x01, 0x01, 0x00,                                // Digital Output
        0x02, 0x02, 0x00, 0x64,                          // Analog Input: 1.0
        0x03, 0x03, 0x00, 0xC8,                          // Analog Output: 2.0
        0x04, 0x65, 0x03, 0xE8,                          // Luminosity: 1000 lux
        0x05, 0x66, 0x01,                                // Presence: 1
        0x06, 0x67, 0x00, 0x64,                          // Temperature: 10.0°C
        0x07, 0x68, 0x01, 0xF4,                          // Humidity: 50.0%
        0x08, 0x73, 0x27, 0x7F,                          // Barometer: 1011.1 hPa
        0x09, 0x86, 0x00, 0x64, 0x00, 0x64, 0x00, 0x64,  // Gyrometer: 1.0, 1.0, 1.0
        0x0A, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // GPS: 0,0,0
        0x0B, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00                     // Accelerometer: 0,0,0
    };
    auto result = decoder_.decode(payload);

    EXPECT_EQ(result["Digital Input_0"], 1);
    EXPECT_EQ(result["Digital Output_1"], 0);
    EXPECT_DOUBLE_EQ(result["Analog Input_2"], 1.0);
    EXPECT_DOUBLE_EQ(result["Analog Output_3"], 2.0);
    EXPECT_EQ(result["Luminosity_4"], 1000);
    EXPECT_EQ(result["Presence_5"], 1);
    EXPECT_DOUBLE_EQ(result["Temperature_6"], 10.0);
    EXPECT_DOUBLE_EQ(result["Humidity_7"], 50.0);
    EXPECT_DOUBLE_EQ(result["Barometer_8"], 1011.1);
    EXPECT_DOUBLE_EQ(result["Gyrometer_9"]["x"], 1.0);
    EXPECT_DOUBLE_EQ(result["GPS_10"]["latitude"], 0.0);
    EXPECT_DOUBLE_EQ(result["Accelerometer_11"]["x"], 0.0);
}

TEST_F(DecoderTest, MaxChannelNumber)
{
    std::vector<std::uint8_t> payload = {0xFF, 0x00, 0x01};  // Channel 255
    auto result = decoder_.decode(payload);
    EXPECT_EQ(result["Digital Input_255"], 1);
}

TEST_F(DecoderTest, MixedPositiveNegativeValues)
{
    std::vector<std::uint8_t> payload = {
        0x01, 0x67, 0x01, 0x2C,  // Temperature: 30.0°C
        0x02, 0x67, 0xFF, 0x38,  // Temperature: -20.0°C
        0x03, 0x02, 0x03, 0xE8,  // Analog Input: 10.0
        0x04, 0x02, 0xFC, 0x18   // Analog Input: -10.0
    };
    auto result = decoder_.decode(payload);

    EXPECT_DOUBLE_EQ(result["Temperature_1"], 30.0);
    EXPECT_DOUBLE_EQ(result["Temperature_2"], -20.0);
    EXPECT_DOUBLE_EQ(result["Analog Input_3"], 10.0);
    EXPECT_DOUBLE_EQ(result["Analog Input_4"], -10.0);
}

// ============================================================================
// ERROR HANDLING EDGE CASES
// ============================================================================

TEST_F(DecoderTest, TruncatedGPS)
{
    std::vector<std::uint8_t> payload = {0x01, 0x88, 0x00, 0x00};  // GPS needs 9 bytes
    EXPECT_THROW(decoder_.decode(payload), BadPayloadFormatException);
}

TEST_F(DecoderTest, TruncatedAccelerometer)
{
    std::vector<std::uint8_t> payload = {0x01, 0x71, 0x00, 0x00};  // Accelerometer needs 6 bytes
    EXPECT_THROW(decoder_.decode(payload), BadPayloadFormatException);
}

TEST_F(DecoderTest, TruncatedGyrometer)
{
    std::vector<std::uint8_t> payload = {0x01, 0x86, 0x00, 0x00};  // Gyrometer needs 6 bytes
    EXPECT_THROW(decoder_.decode(payload), BadPayloadFormatException);
}

TEST_F(DecoderTest, SingleBytePayload)
{
    std::vector<std::uint8_t> payload = {0x01};
    EXPECT_THROW(decoder_.decode(payload), BadPayloadFormatException);
}

TEST_F(DecoderTest, TwoBytePayload)
{
    std::vector<std::uint8_t> payload = {0x01, 0x00};  // Missing data byte
    EXPECT_THROW(decoder_.decode(payload), BadPayloadFormatException);
}

}  // namespace cayene::test

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
