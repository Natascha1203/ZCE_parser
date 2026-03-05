#pragma once
#include <cstdint>
#include <array>

namespace types {

// block: 32 bytes
//  0: double[1]
//  8: int32[3]
// 20: char[12]
struct T31_block32 {
    double d0;
    std::array<int32_t, 3> i;
    std::array<uint8_t, 12> c;

    T31_block32() : d0(0.0), i(), c() {}
};

// 你描述的那种结构（272起始 10个block，后面还有592/628/668/672等）
struct T31 {
    std::array<double, 3>  off_88_d3;     // 88,  len=24
    std::array<int32_t, 2> off_112_i2;    // 112, len=8
    double                 off_120_d1;    // 120, len=8
    std::array<int32_t, 3> off_128_i3;    // 128, len=12
    std::array<double, 8>  off_140_d8;    // 140, len=64

    std::array<int32_t, 2> off_204_i2;    // 204, int[2]
    double                 off_212_d1;    // 212, double[1]

    std::array<int32_t, 2> off_220_i2;    // 220, int[2]
    double                 off_228_d1;    // 228, double[1]

    std::array<uint8_t, 16> off_236_c16;  // 236, char[16]

    std::array<double, 2>  off_252_d2;    // 252, double[2]
    int32_t                off_268_i1;    // 268, int[1]

    std::array<Block32, 10> off_272_blocks; // 272, 10 * 32 = 320

    std::array<uint8_t, 36> off_592_c36;  // 592, char[36]
    std::array<double, 5>   off_628_d5;   // 628, double[5]
    int32_t                 off_668_i1;   // 668, int[1]
    std::array<int8_t, 4>   off_672_i8_4; // 672, int8[4]（注意越界问题）

    T31()
        : off_88_d3()
        , off_112_i2()
        , off_120_d1(0.0)
        , off_128_i3()
        , off_140_d8()
        , off_204_i2()
        , off_212_d1(0.0)
        , off_220_i2()
        , off_228_d1(0.0)
        , off_236_c16()
        , off_252_d2()
        , off_268_i1(0)
        , off_272_blocks()
        , off_592_c36()
        , off_628_d5()
        , off_668_i1(0)
        , off_672_i8_4()
    {}
};

} // namespace types