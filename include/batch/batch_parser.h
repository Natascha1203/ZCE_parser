#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "types/t31.h"

namespace batch {

// 批量解析结果
#if 0
struct ParseResult {
    size_t offset;              // 数据包在原始数据中的偏移
    parser::MsgType type;       // 消息类型
    bool success;               // 解析是否成功
    types::T31 t31_data;        // T31 解析结果（如果是 T31 类型）
    
    ParseResult() : offset(0), type(parser::MsgType::T31), success(false) {}
};
#endif

struct BatchCsvStats {
    size_t detected = 0;
    size_t parsed_ok = 0;
    size_t parsed_fail = 0;
    size_t t31_rows = 0;
    size_t t32_rows = 0;
    std::string t31_csv_path;
    std::string t32_csv_path;
};

struct ParseTypeOptions {
    bool enable_t31 = true;
    bool enable_t32 = true;
};


BatchCsvStats batchParseToCsv(
    const uint8_t* data,
    size_t len,
    const std::string& output_base_csv,
    const ParseTypeOptions& options);

} // namespace batch
