
#ifndef CAYENE_DATA_TYPE_HPP
#define CAYENE_DATA_TYPE_HPP

#include <cstdint>
#include <functional>
#include <span>
#include <string>

#include <nlohmann/json.hpp>

namespace cayene
{

class DataType
{
public:
    std::string name{"None"};
    std::size_t size{0};
    uint8_t type_id{0};
    bool standard{false};
    std::function<nlohmann::json(const std::span<uint8_t>&)> decoder_function;

    DataType(uint8_t type_id, std::string name, std::size_t size, bool standard = true,
             std::function<nlohmann::json(const std::span<uint8_t>&)> decoder_function = nullptr)
        : name(std::move(name)),
          size(size),
          type_id(type_id),
          standard(standard),
          decoder_function(std::move(decoder_function))
    {
    }
};

}  // namespace cayene

#endif  // CAYENE_DATA_TYPE_HPP