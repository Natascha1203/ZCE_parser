#pragma once


#pragma once

#include <cstdint>

namespace types {

#pragma pack(push, 1)
struct QuotT32 {
    double price;
    int32_t volume;
    int32_t implyVolume;
    char direct;
    char genTime[13];
    char FP[2];
};

struct T32 { 
    char type;
    char FP[3];
    uint32_t length;
    char tradingDay[9];
    char instrumentId[83];
    uint32_t tid;
    double lastPrice;
    double lowPrice;
    double highPrice;
    double lifeLow;
    double lifeHigh;
    double upperLimitPrice;
    double lowerLimitPrice;
    double bidPrice;
    uint32_t bidQty;
    double askPrice;
    uint32_t askQty;
    char genTime[9];
    char FP2[3];
    uint32_t priceNum;
    QuotT32 bid[5];
    QuotT32 ask[5];
    char unknown[4];
    uint32_t sequenceNo;
    uint32_t endCode;
};
#pragma pack(pop)

}  // namespace types
