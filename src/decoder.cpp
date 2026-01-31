/**
 * @file decoder.cpp
 * @brief Implementation of the Cayene Decoder
 *
 * This library is licensed under the GNU General Public License v2 (GPLv2).
 * See LICENSE file for details.
 */

#include "cayene/decoder.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

#include "cayene_v1_definitions.hpp"

namespace cayene
{

Decoder::Decoder()
{
    auto standard_data_types = definitions::get_v1_standard_data_types();

    for (auto& data_type : standard_data_types)
    {
        data_types_.emplace(data_type.type_id, std::move(data_type));
    }
}

Decoder::~Decoder() = default;

auto Decoder::decode(std::span<const std::uint8_t> encoded_payload) -> std::expected<Json, Error>
{
    if (encoded_payload.empty())
    {
        return std::unexpected(Error::PayloadEmpty);
    }

    std::size_t current_index = 0;
    Json decoded_json = Json::object();

    while (current_index + 2 <= encoded_payload.size())
    {
        const std::uint8_t channel = encoded_payload[current_index++];
        const std::uint8_t type_id = encoded_payload[current_index++];

        // If the data type is not registered
        if (!data_types_.contains(type_id))
        {
            return std::unexpected(Error::UnknownDataType);
        }

        const DataType& data_type = data_types_.at(type_id);

        // If remaining bytes are less than required by the data type
        if (current_index + data_type.size > encoded_payload.size())
        {
            return std::unexpected(Error::BadPayloadFormat);
        }

        const auto data_span = encoded_payload.subspan(current_index, data_type.size);
        const std::string key = data_type.name + "_" + std::to_string(channel);

        if (!data_type.standard)
        {
            if (data_type.decoder_function)
            {
                decoded_json[key] = data_type.decoder_function(data_span);
            }
            else
            {
                return std::unexpected(Error::Unexpected);
            }
            current_index += data_type.size;
            continue;
        }

        switch (type_id)
        {
            case 0x00:
                decoded_json[key] = decode_digital_input(data_span);
                break;
            case 0x01:
                decoded_json[key] = decode_digital_output(data_span);
                break;
            case 0x02:
                decoded_json[key] = decode_analog_input(data_span);
                break;
            case 0x03:
                decoded_json[key] = decode_analog_output(data_span);
                break;
            case 0x65:
                decoded_json[key] = decode_luminosity(data_span);
                break;
            case 0x66:
                decoded_json[key] = decode_presence(data_span);
                break;
            case 0x67:
                decoded_json[key] = decode_temperature(data_span);
                break;
            case 0x68:
                decoded_json[key] = decode_humidity(data_span);
                break;
            case 0x71:
                decoded_json[key] = decode_accelerometer(data_span);
                break;
            case 0x73:
                decoded_json[key] = decode_barometer(data_span);
                break;
            case 0x86:
                decoded_json[key] = decode_gyrometer(data_span);
                break;
            case 0x88:
                decoded_json[key] = decode_gps(data_span);
                break;
            default:
                return std::unexpected(Error::UnknownDataType);
        }

        current_index += data_type.size;
    }

    // If there are unprocessed bytes remaining
    if (current_index != encoded_payload.size())
    {
        return std::unexpected(Error::BadPayloadFormat);
    }

    return decoded_json;
}

bool Decoder::add_custom_type(std::uint8_t type_id, std::string name, std::size_t size,
                              DecoderFunction decoder_function)
{
    if (data_types_.contains(type_id))
    {
        return false;
    }

    if (!decoder_function)
    {
        return false;
    }

    if (size == 0)
    {
        return false;
    }

    data_types_.emplace(
        type_id, DataType(type_id, std::move(name), size, false, std::move(decoder_function)));
    return true;
}

bool Decoder::has_type(std::uint8_t type_id) const noexcept
{
    return data_types_.contains(type_id);
}

bool Decoder::remove_custom_type(std::uint8_t type_id)
{
    auto iter = data_types_.find(type_id);
    if (iter == data_types_.end())
    {
        return false;
    }

    // Cannot remove standard types
    if (iter->second.standard)
    {
        return false;
    }

    data_types_.erase(iter);
    return true;
}

std::uint16_t Decoder::bytes_to_uint16(std::span<const std::uint8_t> data_span)
{
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(data_span[0]) << 8U) |
                                      static_cast<std::uint16_t>(data_span[1]));
}

std::int16_t Decoder::bytes_to_int16(std::span<const std::uint8_t> data_span)
{
    const std::uint16_t unsigned_value = bytes_to_uint16(data_span);
    // If the value is greater than the max positive int16_t, it's negative
    if (unsigned_value > 0x7FFFU)
    {
        return static_cast<std::int16_t>(static_cast<std::int32_t>(unsigned_value) - 0x10000);
    }

    return static_cast<std::int16_t>(unsigned_value);
}

std::uint32_t Decoder::bytes_to_uint24(std::span<const std::uint8_t> data_span)
{
    return ((static_cast<std::uint32_t>(data_span[0]) << 16U) |
            (static_cast<std::uint32_t>(data_span[1]) << 8U) |
            static_cast<std::uint32_t>(data_span[2])) &
           0x00FFFFFFU;
}

std::int32_t Decoder::bytes_to_int24(std::span<const std::uint8_t> data_span)
{
    const std::uint32_t unsigned_value = bytes_to_uint24(data_span);
    // If the value is greater than the max positive 24-bit int, it's negative
    if (unsigned_value > 0x7FFFFFU)
    {
        return static_cast<std::int32_t>(unsigned_value) - 0x1000000;
    }

    return static_cast<std::int32_t>(unsigned_value);
}

Json Decoder::decode_digital_input(std::span<const std::uint8_t> data_span)
{
    return Json(data_span[0]);
}

Json Decoder::decode_digital_output(std::span<const std::uint8_t> data_span)
{
    return Json(data_span[0]);
}

Json Decoder::decode_analog_input(std::span<const std::uint8_t> data_span)
{
    const std::int16_t raw_value = bytes_to_int16(data_span);
    return Json(raw_value / 100.0);
}

Json Decoder::decode_analog_output(std::span<const std::uint8_t> data_span)
{
    const std::int16_t raw_value = bytes_to_int16(data_span);
    return Json(raw_value / 100.0);
}

Json Decoder::decode_luminosity(std::span<const std::uint8_t> data_span)
{
    return Json(bytes_to_uint16(data_span));
}

Json Decoder::decode_presence(std::span<const std::uint8_t> data_span)
{
    return Json(data_span[0]);
}

Json Decoder::decode_temperature(std::span<const std::uint8_t> data_span)
{
    const std::int16_t raw_value = bytes_to_int16(data_span);
    return Json(raw_value / 10.0);
}

Json Decoder::decode_humidity(std::span<const std::uint8_t> data_span)
{
    const std::uint16_t raw_value = bytes_to_uint16(data_span);
    return Json(raw_value / 10.0);
}

Json Decoder::decode_accelerometer(std::span<const std::uint8_t> data_span)
{
    Json accel_json = Json::object();

    const std::int16_t x_raw = bytes_to_int16(data_span.subspan(0, 2));
    const std::int16_t y_raw = bytes_to_int16(data_span.subspan(2, 2));
    const std::int16_t z_raw = bytes_to_int16(data_span.subspan(4, 2));

    accel_json["x"] = x_raw / 1000.0;
    accel_json["y"] = y_raw / 1000.0;
    accel_json["z"] = z_raw / 1000.0;

    return accel_json;
}

Json Decoder::decode_barometer(std::span<const std::uint8_t> data_span)
{
    const std::uint16_t raw_value = bytes_to_uint16(data_span);
    return Json(raw_value / 10.0);
}

Json Decoder::decode_gyrometer(std::span<const std::uint8_t> data_span)
{
    Json gyro_json = Json::object();

    const std::int16_t x_raw = bytes_to_int16(data_span.subspan(0, 2));
    const std::int16_t y_raw = bytes_to_int16(data_span.subspan(2, 2));
    const std::int16_t z_raw = bytes_to_int16(data_span.subspan(4, 2));

    gyro_json["x"] = x_raw / 100.0;
    gyro_json["y"] = y_raw / 100.0;
    gyro_json["z"] = z_raw / 100.0;

    return gyro_json;
}

Json Decoder::decode_gps(std::span<const std::uint8_t> data_span)
{
    Json gps_json = Json::object();

    const std::int32_t lat_raw = bytes_to_int24(data_span.subspan(0, 3));
    const std::int32_t lon_raw = bytes_to_int24(data_span.subspan(3, 3));
    const std::int32_t alt_raw = bytes_to_int24(data_span.subspan(6, 3));

    gps_json["latitude"] = lat_raw / 10000.0;
    gps_json["longitude"] = lon_raw / 10000.0;
    gps_json["altitude"] = alt_raw / 100.0;

    return gps_json;
}

}  // namespace cayene
