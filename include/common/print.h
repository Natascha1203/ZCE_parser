#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

namespace common {

inline std::string bytes_to_ascii_printable(const uint8_t* p, size_t n) {
    std::string s;
    s.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        char c = static_cast<char>(p[i]);
        s.push_back((c >= 32 && c <= 126) ? c : '.');
    }
    return s;
}

} // namespace common