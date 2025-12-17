#ifndef CAYENE_DECODER_HPP
#define CAYENE_DECODER_HPP

/**
 * @file decoder.hpp
 * @brief Main header for the Cayene Decoder library
 *
 * This library is licensed under the GNU General Public License v2 (GPLv2).
 * See LICENSE file for details.
 */

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <unordered_map>

#include <nlohmann/json.hpp>
#include <sys/types.h>

#include "data_type.hpp"
#include "error.hpp"

namespace cayene
{
using namespace nlohmann;
using Json = nlohmann::json;

class Decoder
{
private:
    std::unordered_map<uint8_t, DataType> data_types_;

public:
    Decoder();
    ~Decoder();

    auto decode(const std::span<uint8_t>& encoded_payload) -> std::expected<Json, Error>;
    void add_data_type(uint8_t type_id, const std::string& name, std::size_t size);

private:
    // Decoding functions for standard data types
    // Is assumed that the data_span passed to these functions has the correct size
    static uint8_t decode_digital_input(const std::span<uint8_t>& data_span);
    static uint8_t decode_digital_output(const std::span<uint8_t>& data_span);
    static double decode_analog_input(const std::span<uint8_t>& data_span);
    static double decode_analog_output(const std::span<uint8_t>& data_span);
    static uint16_t decode_luminosity(const std::span<uint8_t>& data_span);
    static uint8_t decode_presence(const std::span<uint8_t>& data_span);
    static double decode_temperature(const std::span<uint8_t>& data_span);
    static double decode_humidity(const std::span<uint8_t>& data_span);
    static Json decode_accelerometer(const std::span<uint8_t>& data_span);
    static double decode_barometer(const std::span<uint8_t>& data_span);
    static double decode_gyrometer(const std::span<uint8_t>& data_span);
    static Json decode_gps(const std::span<uint8_t>& data_span);
};

}  // namespace cayene

#endif  // CAYENE_DECODER_HPP
