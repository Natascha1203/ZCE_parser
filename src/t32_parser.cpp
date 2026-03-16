#include "parser/parser.h"

#include <cstring>

namespace parser {

bool parseT32(const uint8_t* data, size_t len, types::T32* out) {
    if (!data || !out) return false;
    if (len < sizeof(types::T32)) return false;

    std::memcpy(out, data, sizeof(types::T32));
    return true;
}

}  // namespace parser

