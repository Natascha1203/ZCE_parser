#include "parser/parser.h"

namespace parser {

bool parseByType(MsgType type, const uint8_t* data, size_t len, void* out) {
    if (!out) return false;

    switch (type) {
        case MsgType::T31:
            return parseT31(data, len, reinterpret_cast<types::T31*>(out));
        case MsgType::T32:
            return parseT32(data, len, reinterpret_cast<types::T32*>(out));
        case MsgType::T36:
            return parseT36(data, len, reinterpret_cast<types::T36*>(out));
        default:
            return false;
    }
}

} // namespace parser
