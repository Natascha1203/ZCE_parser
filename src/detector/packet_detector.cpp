#include "detector/packet_detector.h"
#include <cstring>
#include <iostream>

namespace detector {

// 定义所有静态常量
const uint8_t T31Signature::HEADER[8] = {0x31, 0x00, 0x00, 0x00, 0xa4, 0x02, 0x00, 0x00};
const size_t T31Signature::HEADER_LEN;
const size_t T31Signature::PACKET_LEN;
const uint8_t T31Signature::TAIL[4] = {0x0d, 0x0a, 0x00, 0x00};
const size_t T31Signature::TAIL_OFFSET;

// 匹配字节序列
static bool matchBytes(const uint8_t* data, const uint8_t* pattern, size_t len) {
    return std::memcmp(data, pattern, len) == 0;
}

// 验证单个 T31 数据包
bool validateT31Packet(const uint8_t* data, size_t len) {
    // 1. 检查长度是否足够
    if (len < T31Signature::PACKET_LEN) {
        return false;
    }

    // 2. 检查起始标志（8字节）
    if (!matchBytes(data, T31Signature::HEADER, T31Signature::HEADER_LEN)) {
        return false;
    }

    // 3. 检查尾部标志（偏移672处的4字节）
    if (!matchBytes(data + T31Signature::TAIL_OFFSET, 
                    T31Signature::TAIL, 
                    sizeof(T31Signature::TAIL))) {
        return false;
    }

    return true;
}

// 检测所有 T31 数据包
std::vector<PacketInfo> detectT31Packets(const uint8_t* data, size_t len) {
    std::vector<PacketInfo> results;
    
    if (!data || len < T31Signature::PACKET_LEN) {
        return results;
    }

    size_t pos = 0;
    const size_t search_limit = len - T31Signature::PACKET_LEN + 1;

    while (pos < search_limit) {
        // 步骤1: 搜索起始标志的第一个字节（0x31）
        // 快速跳过不匹配的位置
        if (data[pos] != T31Signature::HEADER[0]) {
            pos++;
            continue;
        }

        // 步骤2: 检查完整的起始标志（8字节）
        if (!matchBytes(data + pos, T31Signature::HEADER, T31Signature::HEADER_LEN)) {
            pos++;
            continue;
        }

        // 步骤3: 验证数据包完整性
        if (pos + T31Signature::PACKET_LEN > len) {
            std::cerr << "[WARNING] Incomplete T31 packet at offset " << pos 
                      << " (need " << T31Signature::PACKET_LEN 
                      << " bytes, only " << (len - pos) << " available)\n";
            break;
        }

        // 步骤4: 验证尾部标志（偏移672处）
        const uint8_t* tail_pos = data + pos + T31Signature::TAIL_OFFSET;
        if (!matchBytes(tail_pos, T31Signature::TAIL, sizeof(T31Signature::TAIL))) {
            std::cerr << "[WARNING] T31 header found at offset " << pos 
                      << " but tail validation failed (expected 0d 0a 00 00 at offset " 
                      << (pos + T31Signature::TAIL_OFFSET) << ")\n";
            pos++;
            continue;
        }

        // 步骤5: 验证通过，保存结果
        results.push_back(PacketInfo(pos, parser::MsgType::T31, T31Signature::PACKET_LEN));
        
        std::cout << "[OK] T31 packet detected at offset " << pos 
                  << ", length " << T31Signature::PACKET_LEN << " bytes\n";

        // 步骤6: 跳过已识别的数据包
        pos += T31Signature::PACKET_LEN;
    }

    return results;
}

} // namespace detector
