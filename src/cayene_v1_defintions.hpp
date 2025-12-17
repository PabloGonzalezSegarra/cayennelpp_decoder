

#ifndef CAYENE_V1_DEFINITIONS_HPP
#define CAYENE_V1_DEFINITIONS_HPP

#include <vector>

#include "cayene/data_type.hpp"
namespace cayene::definitions
{

inline std::vector<DataType> get_v1_standard_data_types()
{
    return {
        DataType(0x00, "Digital Input", 1, true), DataType(0x01, "Digital Output", 1, true),
        DataType(0x02, "Analog Input", 2, true),  DataType(0x03, "Analog Output", 2, true),
        DataType(0x65, "Luminosity", 2, true),    DataType(0x66, "Presence", 1, true),
        DataType(0x67, "Temperature", 2, true),   DataType(0x68, "Humidity", 2, true),
        DataType(0x71, "Accelerometer", 6, true), DataType(0x73, "Barometer", 2, true),
        DataType(0x86, "Gyrometer", 6, true),     DataType(0x88, "GPS", 9, true),
    };
}

}  // namespace cayene::definitions

#endif  // CAYENE_V1_DEFINITIONS_HPP