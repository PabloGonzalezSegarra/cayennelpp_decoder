#ifndef CAYENE_DECODER_ERROR_HPP
#define CAYENE_DECODER_ERROR_HPP

/**
 * @file error.hpp
 * @brief Error definitions for the Cayene Decoder library
 *
 * This library is licensed under the GNU General Public License v2 (GPLv2).
 * See LICENSE file for details.
 */

#include <cstdint>

namespace cayene
{

enum class Error : std::uint8_t
{
    None = 0,
    Unexcepted = 1,
    UnkwownDataType = 2,
    BadPayloadFormat = 3,
    PayloadEmpty = 4
};
}

#endif  // CAYENE_DECODER_ERROR_HPP