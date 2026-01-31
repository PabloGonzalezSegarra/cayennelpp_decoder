#ifndef CAYENE_DECODER_ERROR_HPP
#define CAYENE_DECODER_ERROR_HPP

/**
 * @file error.hpp
 * @brief Error definitions for the Cayene Decoder library
 *
 * This library is licensed under the GNU General Public License v2 (GPLv2).
 * See LICENSE file for details.
 */

#include <stdexcept>
#include <string>

namespace cayene
{

/**
 * @brief Base exception for Cayene decoder errors
 */
class DecoderException : public std::runtime_error
{
public:
    explicit DecoderException(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief Exception thrown when payload is empty
 */
class PayloadEmptyException : public DecoderException
{
public:
    PayloadEmptyException() : DecoderException("Payload is empty") {}
};

/**
 * @brief Exception thrown when data type is unknown
 */
class UnknownDataTypeException : public DecoderException
{
public:
    explicit UnknownDataTypeException(unsigned char type)
        : DecoderException("Unknown data type: 0x" + to_hex(type))
    {
    }

private:
    static std::string to_hex(unsigned char c)
    {
        const char* hex = "0123456789ABCDEF";
        return std::string() + hex[c >> 4] + hex[c & 0xF];
    }
};

/**
 * @brief Exception thrown when payload format is invalid
 */
class BadPayloadFormatException : public DecoderException
{
public:
    explicit BadPayloadFormatException(const std::string& reason)
        : DecoderException("Bad payload format: " + reason)
    {
    }
};

/**
 * @brief Exception thrown for unexpected errors
 */
class UnexpectedException : public DecoderException
{
public:
    explicit UnexpectedException(const std::string& reason)
        : DecoderException("Unexpected error: " + reason)
    {
    }
};

}  // namespace cayene

#endif  // CAYENE_DECODER_ERROR_HPP