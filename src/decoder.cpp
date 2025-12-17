/**
 * @file decoder.cpp
 * @brief Implementation of the Cayene Decoder
 *
 * This library is licensed under the GNU General Public License v2 (GPLv2).
 * See LICENSE file for details.
 */

#include "cayene/decoder.hpp"

#include <cstdint>
#include <span>

#include <sys/types.h>

#include "cayene_v1_defintions.hpp"

namespace cayene
{

Decoder::Decoder()
{
    auto standard_data_types = definitions::get_v1_standard_data_types();

    for (const auto& data_type : standard_data_types)
    {
        data_types_.emplace(data_type.type_id, data_type);
    }
}

Decoder::~Decoder() = default;

auto Decoder::decode(const std::span<uint8_t>& encoded_payload) -> std::expected<Json, Error>
{
    if (encoded_payload.size() == 0)
    {
        return {std::unexpected(Error::PayloadEmpty)};
    }

    auto current_index = encoded_payload.begin();
    Json decoded_json = Json::object();

    while (current_index + 2 < encoded_payload.end())
    {
        uint8_t channel = *(current_index++);
        uint8_t type_id = *(current_index++);

        // Si el tipo de dato no está registrado
        if (!data_types_.contains(type_id))
        {
            return {std::unexpected(Error::UnkwownDataType)};
        }

        DataType& data_type = data_types_.at(type_id);
        // Si los bytes restantes son menores que el tamaño requerido por el tipo de dato
        if (current_index + static_cast<std::ptrdiff_t>(data_type.size) > encoded_payload.end())
        {
            return {std::unexpected(Error::BadPayloadFormat)};
        }

        if (!data_type.standard)
        {
            decoded_json[data_type.name + "_" + std::to_string(channel)] =
                data_type.decoder_function(std::span<uint8_t>(current_index, data_type.size));

            current_index += static_cast<std::ptrdiff_t>(data_type.size);
            continue;
        }

        switch (type_id)
        {
            case 0x00:
                decoded_json[data_type.name + "_" + std::to_string(channel)] =
                    decode_digital_input(std::span<uint8_t>(current_index, data_type.size));
                break;
            case 0x01:
                decoded_json[data_type.name + "_" + std::to_string(channel)] =
                    decode_digital_output(std::span<uint8_t>(current_index, data_type.size));
                break;
            case 0x02:
                decoded_json[data_type.name + "_" + std::to_string(channel)] =
                    decode_analog_input(std::span<uint8_t>(current_index, data_type.size));
                break;
            case 0x03:
                decoded_json[data_type.name + "_" + std::to_string(channel)] =
                    decode_analog_output(std::span<uint8_t>(current_index, data_type.size));
                break;
            case 0x65:
                decoded_json[data_type.name + "_" + std::to_string(channel)] =
                    decode_luminosity(std::span<uint8_t>(current_index, data_type.size));
                break;
            case 0x66:
                decoded_json[data_type.name + "_" + std::to_string(channel)] =
                    decode_presence(std::span<uint8_t>(current_index, data_type.size));
                break;
            case 0x67:
                decoded_json[data_type.name + "_" + std::to_string(channel)] =
                    decode_temperature(std::span<uint8_t>(current_index, data_type.size));
                break;
            case 0x68:
                decoded_json[data_type.name + "_" + std::to_string(channel)] =
                    decode_humidity(std::span<uint8_t>(current_index, data_type.size));
                break;
            case 0x71:
                decoded_json[data_type.name + "_" + std::to_string(channel)] =
                    decode_accelerometer(std::span<uint8_t>(current_index, data_type.size));
                break;
            case 0x73:
                decoded_json[data_type.name + "_" + std::to_string(channel)] =
                    decode_barometer(std::span<uint8_t>(current_index, data_type.size));
                break;
            case 0x86:
                decoded_json[data_type.name + "_" + std::to_string(channel)] =
                    decode_gyrometer(std::span<uint8_t>(current_index, data_type.size));
                break;
            case 0x88:
                decoded_json[data_type.name + "_" + std::to_string(channel)] =
                    decode_gps(std::span<uint8_t>(current_index, data_type.size));
                break;
            default:
                return {std::unexpected(Error::UnkwownDataType)};
        }

        current_index += static_cast<std::ptrdiff_t>(data_type.size);
    }

    // Si quedan bytes sin procesar
    if (current_index < encoded_payload.end())
    {
        return {std::unexpected(Error::BadPayloadFormat)};
    }

    return decoded_json;
}

void Decoder::add_data_type(uint8_t type_id, const std::string& name, std::size_t size)
{
    if (!data_types_.contains(type_id))
    {
        data_types_.emplace(type_id, DataType(type_id, name, size));
    }
}

uint8_t Decoder::decode_digital_input(const std::span<uint8_t>& data_span)
{
    return data_span[0];
}

uint8_t Decoder::decode_digital_output(const std::span<uint8_t>& data_span)
{
    return data_span[0];
}

double Decoder::decode_analog_input(const std::span<uint8_t>& data_span)
{
    auto raw_value = static_cast<int16_t>(data_span.at(0) << 8 | data_span.at(1));
    return raw_value / 100.0;
}

double Decoder::decode_analog_output(const std::span<uint8_t>& data_span)
{
    auto raw_value = static_cast<int16_t>(data_span.at(0) << 8 | data_span.at(1));
    return raw_value / 100.0;
}

uint16_t Decoder::decode_luminosity(const std::span<uint8_t>& data_span)
{
    return static_cast<uint16_t>(data_span.at(0) << 8 | data_span.at(1));
}

uint8_t Decoder::decode_presence(const std::span<uint8_t>& data_span)
{
    return data_span[0];
}

double Decoder::decode_temperature(const std::span<uint8_t>& data_span)
{
    auto raw_value = static_cast<int16_t>(data_span.at(0) << 8 | data_span.at(1));
    return raw_value / 10.0;
}

double Decoder::decode_humidity(const std::span<uint8_t>& data_span)
{
    auto raw_value = static_cast<uint16_t>(data_span.at(0) << 8 | data_span.at(1));
    return raw_value / 10.0;
}

Json Decoder::decode_accelerometer(const std::span<uint8_t>& data_span)
{
    Json accel_json = Json::object();

    auto x_raw = static_cast<int16_t>(data_span.at(0) << 8 | data_span.at(1));
    auto y_raw = static_cast<int16_t>(data_span.at(2) << 8 | data_span.at(3));
    auto z_raw = static_cast<int16_t>(data_span.at(4) << 8 | data_span.at(5));

    double x = x_raw / 1000.0;
    double y = y_raw / 1000.0;
    double z = z_raw / 1000.0;

    accel_json["x"] = x;
    accel_json["y"] = y;
    accel_json["z"] = z;

    return accel_json;
}

double Decoder::decode_barometer(const std::span<uint8_t>& data_span)
{
    auto raw_value = static_cast<uint16_t>(data_span.at(0) << 8 | data_span.at(1));
    return raw_value / 10.0;
}

double Decoder::decode_gyrometer(const std::span<uint8_t>& data_span)
{
    auto raw_value = static_cast<int16_t>(data_span.at(0) << 8 | data_span.at(1));
    return raw_value / 100.0;
}

Json Decoder::decode_gps(const std::span<uint8_t>& data_span)
{
    Json gps_json = Json::object();

    auto lat_raw =
        static_cast<int32_t>(data_span.at(0) << 16 | data_span.at(1) << 8 | data_span.at(2));
    auto lon_raw =
        static_cast<int32_t>(data_span.at(3) << 16 | data_span.at(4) << 8 | data_span.at(5));
    auto alt_raw =
        static_cast<int32_t>(data_span.at(6) << 16 | data_span.at(7) << 8 | data_span.at(8));

    double latitude = lat_raw / 10000.0;
    double longitude = lon_raw / 10000.0;
    double altitude = alt_raw / 100.0;

    gps_json["latitude"] = latitude;
    gps_json["longitude"] = longitude;
    gps_json["altitude"] = altitude;

    return gps_json;
}

}  // namespace cayene