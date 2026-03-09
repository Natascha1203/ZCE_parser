#pragma once
#include <vector>
#include "detector/packet_detector.h"
#include "parser/parser.h"
#include "types/t31.h"

namespace batch {

// 批量解析结果
struct ParseResult {
    size_t offset;              // 数据包在原始数据中的偏移
    parser::MsgType type;       // 消息类型
    bool success;               // 解析是否成功
    types::T31 t31_data;        // T31 解析结果（如果是 T31 类型）
    
    ParseResult() : offset(0), type(parser::MsgType::T31), success(false) {}
};

// 批量解析 T31 数据包
std::vector<ParseResult> batchParseT31(const uint8_t* data, size_t len);

} // namespace batch
