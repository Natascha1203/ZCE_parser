#include "parser/parser.h"
#include "common/endian.h"

namespace parser {

static void warn_oob(const char* field, size_t off, size_t need, size_t size) {
    // 你也可以换成日志系统
    (void)field; (void)off; (void)need; (void)size;
}

bool parseT31(const uint8_t* data, size_t len, types::T31* out) {
    if (!data || !out) return false;

    // helper: read double[n]
    auto rd_double_n = [&](size_t off, double* dst, size_t n) -> bool {
        for (size_t i = 0; i < n; ++i) {
            if (!common::read_le<double>(data, len, off + i * 8, dst[i])) return false;
        }
        return true;
    };

    // helper: read int32[n]
    auto rd_i32_n = [&](size_t off, int32_t* dst, size_t n) -> bool {
        for (size_t i = 0; i < n; ++i) {
            if (!common::read_le<int32_t>(data, len, off + i * 4, dst[i])) return false;
        }
        return true;
    };

    // 88: double[3]
    if (!rd_double_n(88, out->off_88_d3.data(), out->off_88_d3.size())) {
        warn_oob("off_88_d3", 88, 24, len);
        return false;
    }

    // 112: int[2]
    if (!rd_i32_n(112, out->off_112_i2.data(), out->off_112_i2.size())) {
        warn_oob("off_112_i2", 112, 8, len);
        return false;
    }

    // 120: double[1]
    if (!common::read_le<double>(data, len, 120, out->off_120_d1)) {
        warn_oob("off_120_d1", 120, 8, len);
        return false;
    }

    // 128: int[3]
    if (!rd_i32_n(128, out->off_128_i3.data(), out->off_128_i3.size())) {
        warn_oob("off_128_i3", 128, 12, len);
        return false;
    }

    // 140: double[8]
    if (!rd_double_n(140, out->off_140_d8.data(), out->off_140_d8.size())) {
        warn_oob("off_140_d8", 140, 64, len);
        return false;
    }

    // 204: int[2], 212: double[1]
    if (!rd_i32_n(204, out->off_204_i2.data(), out->off_204_i2.size())) {
        warn_oob("off_204_i2", 204, 8, len);
        return false;
    }
    if (!common::read_le<double>(data, len, 212, out->off_212_d1)) {
        warn_oob("off_212_d1", 212, 8, len);
        return false;
    }

    // 220: int[2], 228: double[1]
    if (!rd_i32_n(220, out->off_220_i2.data(), out->off_220_i2.size())) {
        warn_oob("off_220_i2", 220, 8, len);
        return false;
    }
    if (!common::read_le<double>(data, len, 228, out->off_228_d1)) {
        warn_oob("off_228_d1", 228, 8, len);
        return false;
    }

    // 236: char[16]
    if (!common::read_bytes(data, len, 236, out->off_236_c16.data(), out->off_236_c16.size())) {
        warn_oob("off_236_c16", 236, 16, len);
        return false;
    }

    // 252: double[2], 268: int[1]
    if (!rd_double_n(252, out->off_252_d2.data(), out->off_252_d2.size())) {
        warn_oob("off_252_d2", 252, 16, len);
        return false;
    }
    if (!common::read_le<int32_t>(data, len, 268, out->off_268_i1)) {
        warn_oob("off_268_i1", 268, 4, len);
        return false;
    }

    // 272: 10 blocks
    for (size_t k = 0; k < out->off_272_blocks.size(); ++k) {
        size_t base = 272 + k * 32;
        types::Block32& b = out->off_272_blocks[k];

        if (!common::read_le<double>(data, len, base + 0, b.d0)) return false;
        if (!rd_i32_n(base + 8, b.i.data(), b.i.size())) return false;
        if (!common::read_bytes(data, len, base + 20, b.c.data(), b.c.size())) return false;
    }

    // 592: char[36]
    if (!common::read_bytes(data, len, 592, out->off_592_c36.data(), out->off_592_c36.size())) {
        warn_oob("off_592_c36", 592, 36, len);
        return false;
    }

    // 628: double[5]
    if (!rd_double_n(628, out->off_628_d5.data(), out->off_628_d5.size())) {
        warn_oob("off_628_d5", 628, 40, len);
        return false;
    }

    // 668: int[1]
    if (!common::read_le<int32_t>(data, len, 668, out->off_668_i1)) {
        warn_oob("off_668_i1", 668, 4, len);
        // 不直接失败也行，看你业务需要：这里我选择失败，保证结构完整
        return false;
    }

    // 672: int8[4]（如果 len==672 会越界）
    // 这里按“尽量解析”策略：越界就把该字段置0，不报错退出
    {
        uint8_t tmp[4] = {0, 0, 0, 0};
        if (common::read_bytes(data, len, 672, tmp, 4)) {
            for (int i = 0; i < 4; ++i) out->off_672_i8_4[i] = static_cast<int8_t>(tmp[i]);
        } else {
            out->off_672_i8_4[0] = out->off_672_i8_4[1] = out->off_672_i8_4[2] = out->off_672_i8_4[3] = 0;
        }
    }

    return true;
}

} // namespace parser