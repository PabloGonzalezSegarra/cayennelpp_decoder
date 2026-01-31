/**
 * @file decoder_test.cpp
 * @brief Unit tests for the Cayene Decoder
 *
 * Tests are organized into the following categories:
 * 1. Error handling
 * 2. Standard types (Digital, Analog, Environmental, Motion, GPS)
 * 3. Multi-sensor payloads
 * 4. Custom type API
 * 5. Custom type decoding
 * 6. Edge cases and boundary conditions
 *
 * This library is licensed under the GNU General Public License v2 (GPLv2).
 * See LICENSE file for details.
 */

#include "cayene/decoder.hpp"

#include <cstdint>
#include <print>
#include <vector>

#include <gtest/gtest.h>

namespace cayene::test
{

// ============================================================================
// Test Fixtures
// ============================================================================

class DecoderTest : public ::testing::Test
{
protected:
    Decoder decoder_;
};

class CustomTypeTest : public ::testing::Test
{
protected:
    Decoder decoder_;

    // Common custom type IDs for testing
    static constexpr std::uint8_t kCounterTypeId = 0xA0;
    static constexpr std::uint8_t kRgbTypeId = 0xA1;
    static constexpr std::uint8_t kBatteryTypeId = 0xA2;
    static constexpr std::uint8_t kStatusTypeId = 0xA3;
    static constexpr std::uint8_t kInt32TypeId = 0xA4;

    void SetUp() override
    {
        // Pre-register commonly used custom types
    }
};

// ============================================================================
// Section 1: Error Handling Tests
// ============================================================================

TEST_F(DecoderTest, DecodeEmptyPayload)
{
    std::vector<std::uint8_t> empty_payload;
    auto res = decoder_.decode(empty_payload);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), Error::PayloadEmpty);
}

TEST_F(DecoderTest, DecodeSingleBytePayload)
{
    std::vector<std::uint8_t> payload = {0x01};
    auto res = decoder_.decode(payload);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), Error::BadPayloadFormat);
}

TEST_F(DecoderTest, DecodeTwoBytePayloadMissingData)
{
    std::vector<std::uint8_t> payload = {0x01, 0x00};  // Channel + type, but no data
    auto res = decoder_.decode(payload);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), Error::BadPayloadFormat);
}

TEST_F(DecoderTest, DecodeIncompleteTemperatureData)
{
    std::vector<std::uint8_t> payload = {0x01, 0x67, 0x01};  // Only 1 byte, needs 2
    auto res = decoder_.decode(payload);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), Error::BadPayloadFormat);
}

TEST_F(DecoderTest, DecodeTrailingBytes)
{
    std::vector<std::uint8_t> payload = {0x01, 0x67, 0x01, 0x10, 0x02};  // Extra byte
    auto res = decoder_.decode(payload);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), Error::BadPayloadFormat);
}

TEST_F(DecoderTest, DecodeUnknownDataType)
{
    std::vector<std::uint8_t> payload = {0x01, 0xFF, 0x00};  // 0xFF is unknown
    auto res = decoder_.decode(payload);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), Error::UnknownDataType);
}

TEST_F(DecoderTest, DecodeIncompleteGpsData)
{
    // GPS needs 9 bytes of data, provide only 5
    std::vector<std::uint8_t> payload = {0x01, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00};
    auto res = decoder_.decode(payload);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), Error::BadPayloadFormat);
}

TEST_F(DecoderTest, DecodeIncompleteAccelerometerData)
{
    // Accelerometer needs 6 bytes, provide only 4
    std::vector<std::uint8_t> payload = {0x01, 0x71, 0x00, 0x00, 0x00, 0x00};
    auto res = decoder_.decode(payload);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), Error::BadPayloadFormat);
}

// ============================================================================
// Section 2: Standard Types - Digital Input/Output
// ============================================================================

TEST_F(DecoderTest, DecodeDigitalInputOne)
{
    std::vector<std::uint8_t> data = {0x01, 0x00, 0x01};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Digital Input_1"], 1);
}

TEST_F(DecoderTest, DecodeDigitalInputZero)
{
    std::vector<std::uint8_t> data = {0x02, 0x00, 0x00};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Digital Input_2"], 0);
}

TEST_F(DecoderTest, DecodeDigitalOutputOne)
{
    std::vector<std::uint8_t> data = {0x01, 0x01, 0x01};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Digital Output_1"], 1);
}

TEST_F(DecoderTest, DecodeDigitalOutputZero)
{
    std::vector<std::uint8_t> data = {0x02, 0x01, 0x00};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Digital Output_2"], 0);
}

// ============================================================================
// Section 2: Standard Types - Analog Input/Output
// ============================================================================

TEST_F(DecoderTest, DecodeAnalogInputPositive)
{
    std::vector<std::uint8_t> data = {0x01, 0x02, 0x0B, 0xB8};  // 3000 -> 30.0
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Analog Input_1"], 30.0);
}

TEST_F(DecoderTest, DecodeAnalogInputNegative)
{
    std::vector<std::uint8_t> data = {0x02, 0x02, 0xFF, 0x9C};  // -100 -> -1.0
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Analog Input_2"], -1.0);
}

TEST_F(DecoderTest, DecodeAnalogOutputPositive)
{
    std::vector<std::uint8_t> data = {0x01, 0x03, 0x0C, 0x80};  // 3200 -> 32.0
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Analog Output_1"], 32.0);
}

TEST_F(DecoderTest, DecodeAnalogOutputNegative)
{
    std::vector<std::uint8_t> data = {0x02, 0x03, 0xFF, 0x38};  // -200 -> -2.0
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Analog Output_2"], -2.0);
}

// ============================================================================
// Section 2: Standard Types - Luminosity
// ============================================================================

TEST_F(DecoderTest, DecodeLuminosityNormal)
{
    std::vector<std::uint8_t> data = {0x01, 0x65, 0x03, 0xE8};  // 1000 lux
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Luminosity_1"], 1000);
}

TEST_F(DecoderTest, DecodeLuminosityHigh)
{
    std::vector<std::uint8_t> data = {0x02, 0x65, 0x27, 0x10};  // 10000 lux
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Luminosity_2"], 10000);
}

TEST_F(DecoderTest, DecodeLuminosityZero)
{
    std::vector<std::uint8_t> data = {0x03, 0x65, 0x00, 0x00};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Luminosity_3"], 0);
}

TEST_F(DecoderTest, DecodeLuminosityMax)
{
    std::vector<std::uint8_t> data = {0x04, 0x65, 0xFF, 0xFF};  // 65535 lux
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Luminosity_4"], 65535);
}

// ============================================================================
// Section 2: Standard Types - Presence
// ============================================================================

TEST_F(DecoderTest, DecodePresenceDetected)
{
    std::vector<std::uint8_t> data = {0x01, 0x66, 0x01};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Presence_1"], 1);
}

TEST_F(DecoderTest, DecodePresenceNotDetected)
{
    std::vector<std::uint8_t> data = {0x02, 0x66, 0x00};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Presence_2"], 0);
}

// ============================================================================
// Section 2: Standard Types - Temperature
// ============================================================================

TEST_F(DecoderTest, DecodeTemperaturePositive)
{
    std::vector<std::uint8_t> data = {0x01, 0x67, 0x01, 0x90};  // 400 -> 40.0C
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_1"], 40.0);
}

TEST_F(DecoderTest, DecodeTemperatureNegative)
{
    std::vector<std::uint8_t> data = {0x02, 0x67, 0xFF, 0x9C};  // -100 -> -10.0C
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_2"], -10.0);
}

TEST_F(DecoderTest, DecodeTemperatureZero)
{
    std::vector<std::uint8_t> data = {0x03, 0x67, 0x00, 0x00};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_3"], 0.0);
}

TEST_F(DecoderTest, DecodeTemperatureMaxPositive)
{
    std::vector<std::uint8_t> data = {0x04, 0x67, 0x7F, 0xFF};  // 32767 -> 3276.7C
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_4"], 3276.7);
}

TEST_F(DecoderTest, DecodeTemperatureMaxNegative)
{
    std::vector<std::uint8_t> data = {0x05, 0x67, 0x80, 0x00};  // -32768 -> -3276.8C
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_5"], -3276.8);
}

// ============================================================================
// Section 2: Standard Types - Humidity
// ============================================================================

TEST_F(DecoderTest, DecodeHumidityNormal)
{
    std::vector<std::uint8_t> data = {0x01, 0x68, 0x02, 0x58};  // 600 -> 60.0%
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Humidity_1"], 60.0);
}

TEST_F(DecoderTest, DecodeHumidityZero)
{
    std::vector<std::uint8_t> data = {0x02, 0x68, 0x00, 0x00};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Humidity_2"], 0.0);
}

TEST_F(DecoderTest, DecodeHumidityMax)
{
    std::vector<std::uint8_t> data = {0x03, 0x68, 0x03, 0xE8};  // 1000 -> 100.0%
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Humidity_3"], 100.0);
}

// ============================================================================
// Section 2: Standard Types - Barometer
// ============================================================================

TEST_F(DecoderTest, DecodeBarometerNormal)
{
    std::vector<std::uint8_t> data = {0x01, 0x73, 0x27, 0x10};  // 10000 -> 1000.0 hPa
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Barometer_1"], 1000.0);
}

TEST_F(DecoderTest, DecodeBarometerZero)
{
    std::vector<std::uint8_t> data = {0x02, 0x73, 0x00, 0x00};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Barometer_2"], 0.0);
}

TEST_F(DecoderTest, DecodeBarometerMax)
{
    std::vector<std::uint8_t> data = {0x03, 0x73, 0xFF, 0xFF};  // 65535 -> 6553.5 hPa
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Barometer_3"], 6553.5);
}

// ============================================================================
// Section 2: Standard Types - Accelerometer
// ============================================================================

TEST_F(DecoderTest, DecodeAccelerometerPositive)
{
    // X=200, Y=100, Z=50 (mG) -> 0.2, 0.1, 0.05 G
    std::vector<std::uint8_t> data = {0x01, 0x71, 0x00, 0xC8, 0x00, 0x64, 0x00, 0x32};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Accelerometer_1"]["x"], 0.2);
    EXPECT_DOUBLE_EQ(res.value()["Accelerometer_1"]["y"], 0.1);
    EXPECT_DOUBLE_EQ(res.value()["Accelerometer_1"]["z"], 0.05);
}

TEST_F(DecoderTest, DecodeAccelerometerNegative)
{
    // X=-200, Y=-100, Z=-50
    std::vector<std::uint8_t> data = {0x02, 0x71, 0xFF, 0x38, 0xFF, 0x9C, 0xFF, 0xCE};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Accelerometer_2"]["x"], -0.2);
    EXPECT_DOUBLE_EQ(res.value()["Accelerometer_2"]["y"], -0.1);
    EXPECT_DOUBLE_EQ(res.value()["Accelerometer_2"]["z"], -0.05);
}

TEST_F(DecoderTest, DecodeAccelerometerMaxValues)
{
    // X=32767, Y=-32768, Z=0
    std::vector<std::uint8_t> data = {0x03, 0x71, 0x7F, 0xFF, 0x80, 0x00, 0x00, 0x00};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Accelerometer_3"]["x"], 32.767);
    EXPECT_DOUBLE_EQ(res.value()["Accelerometer_3"]["y"], -32.768);
    EXPECT_DOUBLE_EQ(res.value()["Accelerometer_3"]["z"], 0.0);
}

TEST_F(DecoderTest, DecodeAccelerometerAllNegativeOne)
{
    // X=-1, Y=-1, Z=-1
    std::vector<std::uint8_t> data = {0x04, 0x71, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Accelerometer_4"]["x"], -0.001);
    EXPECT_DOUBLE_EQ(res.value()["Accelerometer_4"]["y"], -0.001);
    EXPECT_DOUBLE_EQ(res.value()["Accelerometer_4"]["z"], -0.001);
}

// ============================================================================
// Section 2: Standard Types - Gyrometer
// ============================================================================

TEST_F(DecoderTest, DecodeGyrometerPositive)
{
    // vx: 3200, vy: 100, vz: 50 -> 32.0, 1.0, 0.5 deg/s
    std::vector<std::uint8_t> data = {0x01, 0x86, 0x0C, 0x80, 0x00, 0x64, 0x00, 0x32};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_NEAR(res.value()["Gyrometer_1"]["x"], 32.0, 0.01);
    EXPECT_NEAR(res.value()["Gyrometer_1"]["y"], 1.0, 0.01);
    EXPECT_NEAR(res.value()["Gyrometer_1"]["z"], 0.5, 0.01);
}

TEST_F(DecoderTest, DecodeGyrometerNegative)
{
    // vx: -100, vy: -200, vz: -300
    std::vector<std::uint8_t> data = {0x02, 0x86, 0xFF, 0x9C, 0xFF, 0x38, 0xFE, 0xD4};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_NEAR(res.value()["Gyrometer_2"]["x"], -1.0, 0.01);
    EXPECT_NEAR(res.value()["Gyrometer_2"]["y"], -2.0, 0.01);
    EXPECT_NEAR(res.value()["Gyrometer_2"]["z"], -3.0, 0.01);
}

TEST_F(DecoderTest, DecodeGyrometerMaxValues)
{
    // vx: -32768, vy: 32767, vz: -1
    std::vector<std::uint8_t> data = {0x03, 0x86, 0x80, 0x00, 0x7F, 0xFF, 0xFF, 0xFF};
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_NEAR(res.value()["Gyrometer_3"]["x"], -327.68, 0.01);
    EXPECT_NEAR(res.value()["Gyrometer_3"]["y"], 327.67, 0.01);
    EXPECT_NEAR(res.value()["Gyrometer_3"]["z"], -0.01, 0.01);
}

// ============================================================================
// Section 2: Standard Types - GPS
// ============================================================================

namespace
{
std::int32_t double_to_int24(double value, std::uint32_t factor)
{
    return static_cast<std::int32_t>(value * static_cast<double>(factor));
}

std::vector<std::uint8_t> int24_to_bytes(std::int32_t value)
{
    return {static_cast<std::uint8_t>((value >> 16) & 0xFF),
            static_cast<std::uint8_t>((value >> 8) & 0xFF),
            static_cast<std::uint8_t>(value & 0xFF)};
}

std::vector<std::uint8_t> make_gps_payload(std::uint8_t channel, double lat, double lon, double alt)
{
    auto lat_int24 = double_to_int24(lat, 10000);
    auto lon_int24 = double_to_int24(lon, 10000);
    auto alt_int24 = double_to_int24(alt, 100);
    std::vector<std::uint8_t> data = {channel, 0x88};
    auto lat_bytes = int24_to_bytes(lat_int24);
    auto lon_bytes = int24_to_bytes(lon_int24);
    auto alt_bytes = int24_to_bytes(alt_int24);
    data.insert(data.end(), lat_bytes.begin(), lat_bytes.end());
    data.insert(data.end(), lon_bytes.begin(), lon_bytes.end());
    data.insert(data.end(), alt_bytes.begin(), alt_bytes.end());
    return data;
}
}  // anonymous namespace

TEST_F(DecoderTest, DecodeGpsPositiveCoordinates)
{
    auto data = make_gps_payload(0x01, 1.0, 2.0, 12.0);
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["GPS_1"]["latitude"], 1.0);
    EXPECT_DOUBLE_EQ(res.value()["GPS_1"]["longitude"], 2.0);
    EXPECT_DOUBLE_EQ(res.value()["GPS_1"]["altitude"], 12.0);
}

TEST_F(DecoderTest, DecodeGpsNegativeCoordinates)
{
    auto data = make_gps_payload(0x02, -1.0, -2.0, -12.0);
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["GPS_2"]["latitude"], -1.0);
    EXPECT_DOUBLE_EQ(res.value()["GPS_2"]["longitude"], -2.0);
    EXPECT_DOUBLE_EQ(res.value()["GPS_2"]["altitude"], -12.0);
}

TEST_F(DecoderTest, DecodeGpsZeroCoordinates)
{
    auto data = make_gps_payload(0x03, 0.0, 0.0, 0.0);
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["GPS_3"]["latitude"], 0.0);
    EXPECT_DOUBLE_EQ(res.value()["GPS_3"]["longitude"], 0.0);
    EXPECT_DOUBLE_EQ(res.value()["GPS_3"]["altitude"], 0.0);
}

TEST_F(DecoderTest, DecodeGpsArbitraryCoordinates)
{
    auto data = make_gps_payload(0x04, 12.3456, -76.5432, 123.45);
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_NEAR(res.value()["GPS_4"]["latitude"], 12.3456, 0.0001);
    EXPECT_NEAR(res.value()["GPS_4"]["longitude"], -76.5432, 0.0001);
    EXPECT_NEAR(res.value()["GPS_4"]["altitude"], 123.45, 0.01);
}

// ============================================================================
// Section 3: Multi-Sensor Payload Tests
// ============================================================================

TEST_F(DecoderTest, DecodeMultipleSensors)
{
    std::vector<std::uint8_t> data = {
        0x01, 0x67, 0x00, 0xFA,  // Temperature ch1: 25.0C
        0x02, 0x68, 0x02, 0x58   // Humidity ch2: 60.0%
    };
    auto res = decoder_.decode(data);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_1"], 25.0);
    EXPECT_DOUBLE_EQ(res.value()["Humidity_2"], 60.0);
}

TEST_F(DecoderTest, DecodeComplexPayload)
{
    std::vector<std::uint8_t> payload = {
        0x03, 0x67, 0x01, 0x10,                          // Temperature ch3: 27.2C
        0x05, 0x67, 0x00, 0xFF,                          // Temperature ch5: 25.5C
        0x06, 0x71, 0x04, 0xD2, 0xFB, 0x2E, 0x00, 0x00,  // Accelerometer ch6
        0x01, 0x67, 0xFF, 0xD7                           // Temperature ch1: -4.1C
    };
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_3"], 27.2);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_5"], 25.5);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_1"], -4.1);
    EXPECT_TRUE(res.value().contains("Accelerometer_6"));
}

TEST_F(DecoderTest, DecodeSameChannelMultipleTimes)
{
    // Same channel used twice - last value wins
    std::vector<std::uint8_t> payload = {
        0x01, 0x67, 0x00, 0xC8,  // Temperature ch1: 20.0C
        0x01, 0x67, 0x01, 0x2C   // Temperature ch1: 30.0C (overwrites)
    };
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_1"], 30.0);
}

TEST_F(DecoderTest, DecodeAllSensorTypes)
{
    // Payload with one of each sensor type
    std::vector<std::uint8_t> payload = {
        0x01, 0x00, 0x01,        // Digital Input
        0x02, 0x01, 0x00,        // Digital Output
        0x03, 0x02, 0x00, 0x64,  // Analog Input
        0x04, 0x03, 0x00, 0xC8,  // Analog Output
        0x05, 0x65, 0x01, 0xF4,  // Luminosity: 500
        0x06, 0x66, 0x01,        // Presence
        0x07, 0x67, 0x00, 0xFA,  // Temperature: 25.0C
        0x08, 0x68, 0x01, 0xF4,  // Humidity: 50.0%
        0x09, 0x73, 0x27, 0x10   // Barometer: 1000.0 hPa
    };
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Digital Input_1"], 1);
    EXPECT_EQ(res.value()["Digital Output_2"], 0);
    EXPECT_DOUBLE_EQ(res.value()["Analog Input_3"], 1.0);
    EXPECT_DOUBLE_EQ(res.value()["Analog Output_4"], 2.0);
    EXPECT_EQ(res.value()["Luminosity_5"], 500);
    EXPECT_EQ(res.value()["Presence_6"], 1);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_7"], 25.0);
    EXPECT_DOUBLE_EQ(res.value()["Humidity_8"], 50.0);
    EXPECT_DOUBLE_EQ(res.value()["Barometer_9"], 1000.0);
}

// ============================================================================
// Section 4: Custom Type API Tests
// ============================================================================

TEST_F(CustomTypeTest, HasTypeForAllStandardTypes)
{
    EXPECT_TRUE(decoder_.has_type(0x00));  // Digital Input
    EXPECT_TRUE(decoder_.has_type(0x01));  // Digital Output
    EXPECT_TRUE(decoder_.has_type(0x02));  // Analog Input
    EXPECT_TRUE(decoder_.has_type(0x03));  // Analog Output
    EXPECT_TRUE(decoder_.has_type(0x65));  // Luminosity
    EXPECT_TRUE(decoder_.has_type(0x66));  // Presence
    EXPECT_TRUE(decoder_.has_type(0x67));  // Temperature
    EXPECT_TRUE(decoder_.has_type(0x68));  // Humidity
    EXPECT_TRUE(decoder_.has_type(0x71));  // Accelerometer
    EXPECT_TRUE(decoder_.has_type(0x73));  // Barometer
    EXPECT_TRUE(decoder_.has_type(0x86));  // Gyrometer
    EXPECT_TRUE(decoder_.has_type(0x88));  // GPS
}

TEST_F(CustomTypeTest, HasTypeForNonExistent)
{
    EXPECT_FALSE(decoder_.has_type(0xFF));
    EXPECT_FALSE(decoder_.has_type(0xAB));
    EXPECT_FALSE(decoder_.has_type(0x50));
    EXPECT_FALSE(decoder_.has_type(kCounterTypeId));
}

TEST_F(CustomTypeTest, AddCustomTypeSuccess)
{
    bool added = decoder_.add_custom_type(
        kCounterTypeId, "Counter", 2,
        [](std::span<const std::uint8_t> data) -> Json
        {
            auto value = static_cast<std::uint16_t>((static_cast<unsigned>(data[0]) << 8U) |
                                                    static_cast<unsigned>(data[1]));
            return Json(value);
        });

    EXPECT_TRUE(added);
    EXPECT_TRUE(decoder_.has_type(kCounterTypeId));
}

TEST_F(CustomTypeTest, AddCustomTypeFailsForExistingStandardType)
{
    bool added = decoder_.add_custom_type(0x67,  // Temperature type ID
                                          "CustomTemp", 2, [](std::span<const std::uint8_t>) -> Json
                                          { return Json(0); });

    EXPECT_FALSE(added);
}

TEST_F(CustomTypeTest, AddCustomTypeFailsForExistingCustomType)
{
    // First add
    decoder_.add_custom_type(kCounterTypeId, "Counter", 2,
                             [](std::span<const std::uint8_t>) -> Json { return Json(0); });

    // Second add with same ID
    bool added =
        decoder_.add_custom_type(kCounterTypeId, "Counter2", 2,
                                 [](std::span<const std::uint8_t>) -> Json { return Json(1); });

    EXPECT_FALSE(added);
}

TEST_F(CustomTypeTest, AddCustomTypeFailsWithNullDecoder)
{
    bool added = decoder_.add_custom_type(kCounterTypeId, "NullDecoder", 1, nullptr);
    EXPECT_FALSE(added);
    EXPECT_FALSE(decoder_.has_type(kCounterTypeId));
}

TEST_F(CustomTypeTest, AddCustomTypeFailsWithZeroSize)
{
    bool added =
        decoder_.add_custom_type(kCounterTypeId, "ZeroSize", 0,
                                 [](std::span<const std::uint8_t>) -> Json { return Json(0); });

    EXPECT_FALSE(added);
    EXPECT_FALSE(decoder_.has_type(kCounterTypeId));
}

TEST_F(CustomTypeTest, AddCustomTypeWithEmptyName)
{
    // Empty name should still work - the type is identified by ID
    bool added = decoder_.add_custom_type(kCounterTypeId, "", 1,
                                          [](std::span<const std::uint8_t> data) -> Json
                                          { return Json(data[0]); });

    EXPECT_TRUE(added);
    EXPECT_TRUE(decoder_.has_type(kCounterTypeId));
}

TEST_F(CustomTypeTest, RemoveCustomTypeSuccess)
{
    decoder_.add_custom_type(kCounterTypeId, "Counter", 1,
                             [](std::span<const std::uint8_t> data) -> Json
                             { return Json(data[0]); });

    EXPECT_TRUE(decoder_.has_type(kCounterTypeId));

    bool removed = decoder_.remove_custom_type(kCounterTypeId);
    EXPECT_TRUE(removed);
    EXPECT_FALSE(decoder_.has_type(kCounterTypeId));
}

TEST_F(CustomTypeTest, RemoveCustomTypeFailsForStandardType)
{
    bool removed = decoder_.remove_custom_type(0x67);  // Temperature
    EXPECT_FALSE(removed);
    EXPECT_TRUE(decoder_.has_type(0x67));
}

TEST_F(CustomTypeTest, RemoveCustomTypeFailsForNonExistent)
{
    bool removed = decoder_.remove_custom_type(0xFF);
    EXPECT_FALSE(removed);
}

TEST_F(CustomTypeTest, RemoveAndReaddCustomType)
{
    // Add
    decoder_.add_custom_type(kCounterTypeId, "Counter", 1,
                             [](std::span<const std::uint8_t>) -> Json { return Json(1); });

    // Remove
    decoder_.remove_custom_type(kCounterTypeId);
    EXPECT_FALSE(decoder_.has_type(kCounterTypeId));

    // Re-add with different implementation
    bool added =
        decoder_.add_custom_type(kCounterTypeId, "Counter2", 1,
                                 [](std::span<const std::uint8_t>) -> Json { return Json(2); });

    EXPECT_TRUE(added);
    EXPECT_TRUE(decoder_.has_type(kCounterTypeId));
}

// ============================================================================
// Section 5: Custom Type Decoding Tests
// ============================================================================

TEST_F(CustomTypeTest, DecodeSimpleCustomType)
{
    decoder_.add_custom_type(
        kCounterTypeId, "Counter", 2,
        [](std::span<const std::uint8_t> data) -> Json
        {
            auto value = static_cast<std::uint16_t>((static_cast<unsigned>(data[0]) << 8U) |
                                                    static_cast<unsigned>(data[1]));
            return Json(value);
        });

    std::vector<std::uint8_t> payload = {0x01, kCounterTypeId, 0x12, 0x34};
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Counter_1"], 0x1234);
}

TEST_F(CustomTypeTest, DecodeCustomTypeReturningJsonObject)
{
    decoder_.add_custom_type(kRgbTypeId, "RGB", 3,
                             [](std::span<const std::uint8_t> data) -> Json
                             {
                                 Json color = Json::object();
                                 color["r"] = static_cast<int>(data[0]);
                                 color["g"] = static_cast<int>(data[1]);
                                 color["b"] = static_cast<int>(data[2]);
                                 return color;
                             });

    std::vector<std::uint8_t> payload = {0x01, kRgbTypeId, 0xFF, 0x80, 0x00};
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["RGB_1"]["r"], 255);
    EXPECT_EQ(res.value()["RGB_1"]["g"], 128);
    EXPECT_EQ(res.value()["RGB_1"]["b"], 0);
}

TEST_F(CustomTypeTest, DecodeCustomTypeReturningJsonArray)
{
    decoder_.add_custom_type(kCounterTypeId, "ByteArray", 4,
                             [](std::span<const std::uint8_t> data) -> Json
                             {
                                 Json arr = Json::array();
                                 for (std::uint8_t byte : data)
                                 {
                                     arr.push_back(static_cast<int>(byte));
                                 }
                                 return arr;
                             });

    std::vector<std::uint8_t> payload = {0x01, kCounterTypeId, 0x01, 0x02, 0x03, 0x04};
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["ByteArray_1"][0], 1);
    EXPECT_EQ(res.value()["ByteArray_1"][1], 2);
    EXPECT_EQ(res.value()["ByteArray_1"][2], 3);
    EXPECT_EQ(res.value()["ByteArray_1"][3], 4);
}

TEST_F(CustomTypeTest, DecodeCustomTypeSingleByte)
{
    decoder_.add_custom_type(kStatusTypeId, "Status", 1,
                             [](std::span<const std::uint8_t> data) -> Json
                             {
                                 Json status = Json::object();
                                 status["bit0"] = (data[0] & 0x01U) != 0;
                                 status["bit1"] = (data[0] & 0x02U) != 0;
                                 status["bit2"] = (data[0] & 0x04U) != 0;
                                 status["bit3"] = (data[0] & 0x08U) != 0;
                                 return status;
                             });

    std::vector<std::uint8_t> payload = {0x01, kStatusTypeId, 0x0F};
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_TRUE(res.value()["Status_1"]["bit0"]);
    EXPECT_TRUE(res.value()["Status_1"]["bit1"]);
    EXPECT_TRUE(res.value()["Status_1"]["bit2"]);
    EXPECT_TRUE(res.value()["Status_1"]["bit3"]);
}

TEST_F(CustomTypeTest, DecodeCustomTypeSigned32Bit)
{
    decoder_.add_custom_type(
        kInt32TypeId, "SignedInt32", 4,
        [](std::span<const std::uint8_t> data) -> Json
        {
            auto value = static_cast<std::int32_t>((static_cast<std::uint32_t>(data[0]) << 24U) |
                                                   (static_cast<std::uint32_t>(data[1]) << 16U) |
                                                   (static_cast<std::uint32_t>(data[2]) << 8U) |
                                                   static_cast<std::uint32_t>(data[3]));
            return Json(value);
        });

    // Test positive value
    std::vector<std::uint8_t> payload = {0x01, kInt32TypeId, 0x00, 0x01, 0x00, 0x00};  // 65536
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["SignedInt32_1"], 65536);

    // Test negative value
    payload = {0x02, kInt32TypeId, 0xFF, 0xFF, 0xFF, 0xFF};  // -1
    res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["SignedInt32_2"], -1);
}

TEST_F(CustomTypeTest, DecodeCustomTypeMixedWithStandard)
{
    decoder_.add_custom_type(
        kBatteryTypeId, "BatteryVoltage", 2,
        [](std::span<const std::uint8_t> data) -> Json
        {
            auto millivolts = static_cast<std::uint16_t>((static_cast<unsigned>(data[0]) << 8U) |
                                                         static_cast<unsigned>(data[1]));
            return Json(millivolts / 1000.0);
        });

    std::vector<std::uint8_t> payload = {
        0x01, 0x67,           0x00, 0xFA,  // Temperature: 25.0C
        0x02, kBatteryTypeId, 0x0C, 0xE4,  // Battery: 3300mV -> 3.3V
        0x03, 0x68,           0x02, 0x58   // Humidity: 60.0%
    };

    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_1"], 25.0);
    EXPECT_DOUBLE_EQ(res.value()["BatteryVoltage_2"], 3.3);
    EXPECT_DOUBLE_EQ(res.value()["Humidity_3"], 60.0);
}

TEST_F(CustomTypeTest, DecodeMultipleCustomTypes)
{
    decoder_.add_custom_type(
        kBatteryTypeId, "Battery", 2,
        [](std::span<const std::uint8_t> data) -> Json
        {
            auto mv = static_cast<std::uint16_t>((static_cast<unsigned>(data[0]) << 8U) |
                                                 static_cast<unsigned>(data[1]));
            return Json(mv);
        });

    decoder_.add_custom_type(kRgbTypeId, "LED", 3,
                             [](std::span<const std::uint8_t> data) -> Json
                             {
                                 Json led = Json::object();
                                 led["r"] = static_cast<int>(data[0]);
                                 led["g"] = static_cast<int>(data[1]);
                                 led["b"] = static_cast<int>(data[2]);
                                 return led;
                             });

    decoder_.add_custom_type(kStatusTypeId, "Status", 1,
                             [](std::span<const std::uint8_t> data) -> Json
                             { return Json(static_cast<int>(data[0])); });

    std::vector<std::uint8_t> payload = {
        0x01, kBatteryTypeId, 0x0D, 0x48,        // Battery: 3400mV
        0x02, kRgbTypeId,     0x00, 0xFF, 0x00,  // LED: Green
        0x03, kStatusTypeId,  0x07               // Status: 7
    };

    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Battery_1"], 3400);
    EXPECT_EQ(res.value()["LED_2"]["r"], 0);
    EXPECT_EQ(res.value()["LED_2"]["g"], 255);
    EXPECT_EQ(res.value()["LED_2"]["b"], 0);
    EXPECT_EQ(res.value()["Status_3"], 7);
}

TEST_F(CustomTypeTest, DecodeAfterRemovingCustomType)
{
    decoder_.add_custom_type(kCounterTypeId, "Counter", 2,
                             [](std::span<const std::uint8_t>) -> Json { return Json(0); });

    std::vector<std::uint8_t> payload = {0x01, kCounterTypeId, 0x00, 0x00};

    // Should work before removal
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);

    // Remove and try again
    decoder_.remove_custom_type(kCounterTypeId);
    res = decoder_.decode(payload);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), Error::UnknownDataType);
}

TEST_F(CustomTypeTest, DecodeCustomTypeMultipleChannels)
{
    decoder_.add_custom_type(
        kCounterTypeId, "Counter", 2,
        [](std::span<const std::uint8_t> data) -> Json
        {
            auto value = static_cast<std::uint16_t>((static_cast<unsigned>(data[0]) << 8U) |
                                                    static_cast<unsigned>(data[1]));
            return Json(value);
        });

    std::vector<std::uint8_t> payload = {
        0x01, kCounterTypeId, 0x00, 0x01,  // Counter ch1: 1
        0x02, kCounterTypeId, 0x00, 0x02,  // Counter ch2: 2
        0x03, kCounterTypeId, 0x00, 0x03   // Counter ch3: 3
    };

    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Counter_1"], 1);
    EXPECT_EQ(res.value()["Counter_2"], 2);
    EXPECT_EQ(res.value()["Counter_3"], 3);
}

TEST_F(CustomTypeTest, DecodeCustomTypeReturningString)
{
    decoder_.add_custom_type(kCounterTypeId, "DeviceId", 4,
                             [](std::span<const std::uint8_t> data) -> Json
                             {
                                 char id[9];
                                 std::snprintf(id, sizeof(id), "%02X%02X%02X%02X", data[0], data[1],
                                               data[2], data[3]);
                                 return Json(id);
                             });

    std::vector<std::uint8_t> payload = {0x01, kCounterTypeId, 0xAB, 0xCD, 0xEF, 0x12};
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["DeviceId_1"], "ABCDEF12");
}

TEST_F(CustomTypeTest, DecodeCustomTypeReturningBoolean)
{
    decoder_.add_custom_type(kStatusTypeId, "Active", 1,
                             [](std::span<const std::uint8_t> data) -> Json
                             { return Json(data[0] != 0); });

    std::vector<std::uint8_t> payload_true = {0x01, kStatusTypeId, 0x01};
    auto res = decoder_.decode(payload_true);
    ASSERT_TRUE(res);
    EXPECT_TRUE(res.value()["Active_1"]);

    std::vector<std::uint8_t> payload_false = {0x02, kStatusTypeId, 0x00};
    res = decoder_.decode(payload_false);
    ASSERT_TRUE(res);
    EXPECT_FALSE(res.value()["Active_2"]);
}

TEST_F(CustomTypeTest, DecodeCustomTypeLargeSize)
{
    // Test with 16-byte custom type (like a UUID)
    constexpr std::uint8_t kUuidTypeId = 0xB0;

    decoder_.add_custom_type(
        kUuidTypeId, "UUID", 16,
        [](std::span<const std::uint8_t> data) -> Json
        {
            char uuid[37];
            std::snprintf(uuid, sizeof(uuid),
                          "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                          data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
                          data[8], data[9], data[10], data[11], data[12], data[13], data[14],
                          data[15]);
            return Json(uuid);
        });

    std::vector<std::uint8_t> payload = {0x01, kUuidTypeId, 0x12, 0x34, 0x56, 0x78,
                                         0x9A, 0xBC,        0xDE, 0xF0, 0x11, 0x22,
                                         0x33, 0x44,        0x55, 0x66, 0x77, 0x88};

    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["UUID_1"], "12345678-9abc-def0-1122-334455667788");
}

// ============================================================================
// Section 6: Edge Cases and Boundary Conditions
// ============================================================================

TEST_F(DecoderTest, DecodeChannelZero)
{
    std::vector<std::uint8_t> payload = {0x00, 0x67, 0x00, 0xC8};  // Channel 0
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_0"], 20.0);
}

TEST_F(DecoderTest, DecodeMaxChannel)
{
    std::vector<std::uint8_t> payload = {0xFF, 0x67, 0x00, 0xC8};  // Channel 255
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_DOUBLE_EQ(res.value()["Temperature_255"], 20.0);
}

TEST_F(DecoderTest, DecodeExactMinimalPayload)
{
    // Minimal valid payload: digital input (1 byte data)
    std::vector<std::uint8_t> payload = {0x00, 0x00, 0x01};
    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value()["Digital Input_0"], 1);
}

TEST_F(DecoderTest, DecodeMaximumSizePayload)
{
    // Create a large payload with many sensors
    std::vector<std::uint8_t> payload;
    for (std::uint8_t ch = 0; ch < 50; ++ch)
    {
        payload.push_back(ch);    // Channel
        payload.push_back(0x67);  // Temperature type
        payload.push_back(0x00);  // High byte
        payload.push_back(ch);    // Low byte (channel number as value)
    }

    auto res = decoder_.decode(payload);
    ASSERT_TRUE(res);
    EXPECT_EQ(res.value().size(), 50);

    for (std::uint8_t ch = 0; ch < 50; ++ch)
    {
        std::string key = "Temperature_" + std::to_string(ch);
        EXPECT_DOUBLE_EQ(res.value()[key], ch / 10.0);
    }
}

TEST_F(DecoderTest, DecodeDecoderInstanceIsolation)
{
    // Verify that two decoder instances are independent
    Decoder decoder1;
    Decoder decoder2;

    decoder1.add_custom_type(0xA0, "Custom1", 1,
                             [](std::span<const std::uint8_t>) -> Json { return Json(1); });

    decoder2.add_custom_type(0xA1, "Custom2", 1,
                             [](std::span<const std::uint8_t>) -> Json { return Json(2); });

    EXPECT_TRUE(decoder1.has_type(0xA0));
    EXPECT_FALSE(decoder1.has_type(0xA1));

    EXPECT_FALSE(decoder2.has_type(0xA0));
    EXPECT_TRUE(decoder2.has_type(0xA1));
}

}  // namespace cayene::test
