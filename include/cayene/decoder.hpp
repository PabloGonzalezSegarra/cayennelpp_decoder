#ifndef CAYENE_DECODER_HPP
#define CAYENE_DECODER_HPP

/**
 * @file decoder.hpp
 * @brief Main header for the Cayene Decoder library
 *
 * This library is licensed under the GNU General Public License v2 (GPLv2).
 * See LICENSE file for details.
 */

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "data_type.hpp"
#include "error.hpp"

namespace cayene
{

using Json = nlohmann::json;

/**
 * @brief Decoder function type for custom data types
 *
 * Takes a span of bytes and returns a JSON value.
 */
using DecoderFunction = std::function<Json(std::span<const std::uint8_t>)>;

/**
 * @brief Cayene LPP decoder
 *
 * Decodes Cayene LPP (Low Power Payload) format into JSON.
 * Supports all standard data types and allows registering custom types.
 */
class Decoder
{
public:
    Decoder();
    ~Decoder();

    // Non-copyable but moveable
    Decoder(const Decoder&) = delete;
    Decoder& operator=(const Decoder&) = delete;
    Decoder(Decoder&&) noexcept = default;
    Decoder& operator=(Decoder&&) noexcept = default;

    /**
     * @brief Decode a Cayene LPP encoded payload
     *
     * @param encoded_payload The raw payload bytes to decode
     * @return Decoded JSON object
     * @throws PayloadEmptyException if payload is empty
     * @throws UnknownDataTypeException if unknown data type encountered
     * @throws BadPayloadFormatException if payload format is invalid
     */
    [[nodiscard]] auto decode(std::span<const std::uint8_t> encoded_payload) -> Json;

    /**
     * @brief Register a custom data type
     *
     * @param type_id The type identifier (should not conflict with standard types)
     * @param name Human-readable name for the data type
     * @param size Number of bytes this type consumes
     * @param decoder_function Function to decode the bytes into JSON
     * @return true if registration succeeded, false if type_id already exists
     */
    bool add_custom_type(std::uint8_t type_id, std::string name, std::size_t size,
                         DecoderFunction decoder_function);

    /**
     * @brief Check if a data type is registered
     *
     * @param type_id The type identifier to check
     * @return true if the type is registered
     */
    [[nodiscard]] bool has_type(std::uint8_t type_id) const noexcept;

    /**
     * @brief Remove a custom data type
     *
     * @param type_id The type identifier to remove
     * @return true if the type was removed, false if it was a standard type or didn't exist
     */
    bool remove_custom_type(std::uint8_t type_id);

private:
    std::unordered_map<std::uint8_t, DataType> data_types_;

    // Byte conversion utilities
    [[nodiscard]] static std::int16_t bytes_to_int16(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static std::uint16_t bytes_to_uint16(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static std::int32_t bytes_to_int24(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static std::uint32_t bytes_to_uint24(std::span<const std::uint8_t> data_span);

    // Standard type decoders
    [[nodiscard]] static Json decode_digital_input(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static Json decode_digital_output(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static Json decode_analog_input(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static Json decode_analog_output(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static Json decode_luminosity(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static Json decode_presence(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static Json decode_temperature(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static Json decode_humidity(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static Json decode_accelerometer(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static Json decode_barometer(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static Json decode_gyrometer(std::span<const std::uint8_t> data_span);
    [[nodiscard]] static Json decode_gps(std::span<const std::uint8_t> data_span);
};

}  // namespace cayene

#endif  // CAYENE_DECODER_HPP
