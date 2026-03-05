#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <algorithm>

namespace common {

inline bool is_little_endian_host() {
    uint16_t x = 1;
    return *reinterpret_cast<uint8_t*>(&x) == 1;
}

template <typename T>
inline bool read_le(const uint8_t* data, size_t len, size_t off, T& out) {
    if (!data) return false;
    if (off + sizeof(T) > len) return false;

    std::memcpy(&out, data + off, sizeof(T));

    if (!is_little_endian_host()) {
        uint8_t* p = reinterpret_cast<uint8_t*>(&out);
        for (size_t i = 0, j = sizeof(T) - 1; i < j; ++i, --j) std::swap(p[i], p[j]);
    }
    return true;
}

inline bool read_bytes(const uint8_t* data, size_t len, size_t off, uint8_t* out, size_t n) {
    if (!data || !out) return false;
    if (off + n > len) return false;
    std::memcpy(out, data + off, n);
    return true;
}

} // namespace common