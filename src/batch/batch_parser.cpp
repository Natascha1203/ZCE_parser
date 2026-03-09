#include "batch/batch_parser.h"
#include <iostream>

namespace batch {

std::vector<ParseResult> batchParseT31(const uint8_t* data, size_t len) {
    std::vector<ParseResult> results;

    // 步骤1: 检测所有 T31 数据包
    std::cout << "Detecting T31 packets...\n";
    auto packets = detector::detectT31Packets(data, len);
    
    if (packets.empty()) {
        std::cout << "No T31 packets found.\n";
        return results;
    }

    std::cout << "Found " << packets.size() << " T31 packet(s)\n\n";

    // 步骤2: 遍历并解析每个数据包
    for (size_t i = 0; i < packets.size(); ++i) {
        const auto& pkt = packets[i];
        ParseResult result;
        result.offset = pkt.offset;
        result.type = pkt.type;

        std::cout << "Parsing packet #" << (i + 1) << " at offset " << pkt.offset << "...\n";

        // 步骤3: 调用对应的解析器
        if (pkt.type == parser::MsgType::T31) {
            result.success = parser::parseT31(data + pkt.offset, pkt.length, &result.t31_data);
        } else {
            result.success = false;
        }

        if (result.success) {
            std::cout << "  [OK] Parsed successfully\n";
        } else {
            std::cerr << "  [ERROR] Parse failed\n";
        }

        results.push_back(result);
    }

    return results;
}

} // namespace batch
