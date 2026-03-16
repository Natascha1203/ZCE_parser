#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

#include "parser/parser.h"

namespace detector {

// 数据包信息
struct PacketInfo {
    size_t offset;          // 数据包在缓冲区中的起始位置
    parser::MsgType type;   // 消息类型
    size_t length;          // 数据包长度

    PacketInfo(size_t off, parser::MsgType t, size_t len)
        : offset(off), type(t), length(len) {}
};

// T31 数据包规则
struct T31Signature {
    static const uint8_t HEADER[8];
    static const size_t HEADER_LEN = 8;
    static const size_t PACKET_LEN = 676;
    static const uint8_t TAIL[4];
    static const size_t TAIL_OFFSET = 672;  // 从起始位置算起
};

struct T32Signature {
    static const uint8_t HEADER[8];
    static const size_t HEADER_LEN = 8;
    static const size_t PACKET_LEN = 532;
    static const uint8_t TAIL[4];
    static const size_t TAIL_OFFSET = 528;
};

// 检测所有 T31 数据包
std::vector<PacketInfo> detectT31Packets(const uint8_t* data, size_t len);
std::vector<PacketInfo> detectPackets(const uint8_t* data, size_t len);

// 验证单个 T31 数据包
bool validateT31Packet(const uint8_t* data, size_t len);
bool validateT32Packet(const uint8_t* data, size_t len);

} // namespace detector
