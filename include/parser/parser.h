#pragma once
#include <cstddef>
#include <cstdint>

#include "types/t31.h"
#include "types/t32.h"
#include "types/t36.h"

namespace parser {

// 每种类型一个独立解析函数：输入起始地址 data + 长度 len，输出到 out
bool parseT31(const uint8_t* data, size_t len, types::T31* out);
bool parseT32(const uint8_t* data, size_t len, types::T32* out);
bool parseT36(const uint8_t* data, size_t len, types::T36* out);

// （可选）统一分发接口：后续你有"消息类型ID"时可以用这个
enum class MsgType : uint32_t {
    T31 = 672, // 仅示例
    T32 = 532,
    T36 = 116,
};

bool parseByType(MsgType type, const uint8_t* data, size_t len, void* out);

} // namespace parser
