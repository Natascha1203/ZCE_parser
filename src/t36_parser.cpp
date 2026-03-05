#include "parser/parser.h"
#include "common/endian.h"

namespace parser {

static void warn_oob(const char* field, size_t off, size_t need, size_t size) {
    // 可以换成日志系统
    (void)field; (void)off; (void)need; (void)size;
}

bool parseT36(const uint8_t* data, size_t len, types::T36* out) {
    if (!data || !out) return false;

    // helper: read int32[n]
    auto rd_i32_n = [&](size_t off, int32_t* dst, size_t n) -> bool {
        for (size_t i = 0; i < n; ++i) {
            if (!common::read_le<int32_t>(data, len, off + i * 4, dst[i])) return false;
        }
        return true;
    };

    // helper: read double[n]
    auto rd_double_n = [&](size_t off, double* dst, size_t n) -> bool {
        for (size_t i = 0; i < n; ++i) {
            if (!common::read_le<double>(data, len, off + i * 8, dst[i])) return false;
        }
        return true;
    };

    // 4: char[84]
    if (!common::read_bytes(data, len, 4, out->off_4_c84.data(), out->off_4_c84.size())) {
        warn_oob("off_4_c84", 4, 84, len);
        return false;
    }

    // 88: int[2]
    if (!rd_i32_n(88, out->off_88_i2.data(), out->off_88_i2.size())) {
        warn_oob("off_88_i2", 88, 8, len);
        return false;
    }

    // 96: double[2]
    if (!rd_double_n(96, out->off_96_d2.data(), out->off_96_d2.size())) {
        warn_oob("off_96_d2", 96, 16, len);
        return false;
    }

    // 112: int8[4]
    {
        uint8_t tmp[4] = {0, 0, 0, 0};
        if (common::read_bytes(data, len, 112, tmp, 4)) {
            for (int i = 0; i < 4; ++i) out->off_112_i8_4[i] = static_cast<int8_t>(tmp[i]);
        } else {
            warn_oob("off_112_i8_4", 112, 4, len);
            return false;
        }
    }

    return true;
}

} // namespace parser
