#ifndef CAYENE_DATA_TYPE_HPP
#define CAYENE_DATA_TYPE_HPP

/**
 * @file data_type.hpp
 * @brief Data type definitions for the Cayene Decoder library
 *
 * This library is licensed under the GNU General Public License v2 (GPLv2).
 * See LICENSE file for details.
 */

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

namespace cayene
{

/**
 * @brief Represents a Cayene LPP data type
 *
 * Contains metadata and an optional decoder function for custom types.
 */
class DataType
{
public:
    std::string name;
    std::size_t size{0};
    std::uint8_t type_id{0};
    bool standard{false};
    std::function<nlohmann::json(std::span<const std::uint8_t>)> decoder_function;

    DataType() = default;

    DataType(std::uint8_t type_id_arg, std::string name_arg, std::size_t size_arg,
             bool standard_arg = true,
             std::function<nlohmann::json(std::span<const std::uint8_t>)> decoder_func = nullptr)
        : name(std::move(name_arg)),
          size(size_arg),
          type_id(type_id_arg),
          standard(standard_arg),
          decoder_function(std::move(decoder_func))
    {
    }
};

}  // namespace cayene

#endif  // CAYENE_DATA_TYPE_HPP