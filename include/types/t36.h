#pragma once
#include <cstdint>
#include <array>

namespace types {

// T36 structure: total 116 bytes
// offset=4,  len=84, char[84]
// offset=88, len=8,  int[2]
// offset=96, len=16, double[2]
// offset=112,len=4,  int8[4]
struct T36 {
    std::array<uint8_t, 84> off_4_c84;    // 4,   char[84]
    std::array<int32_t, 2>  off_88_i2;    // 88,  int[2]
    std::array<double, 2>   off_96_d2;    // 96,  double[2]
    std::array<int8_t, 4>   off_112_i8_4; // 112, int8[4]

    T36()
        : off_4_c84()
        , off_88_i2()
        , off_96_d2()
        , off_112_i8_4()
    {}
};

} // namespace types
