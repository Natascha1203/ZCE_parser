#include "t_printer.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <ostream>
#include <string>

namespace printer {
namespace {

void write_float(std::ostream& out, double v) {
    const double abs_v = std::fabs(v);

    if (std::isfinite(v) && abs_v >= 1e12) {
        out << std::scientific << std::setprecision(17) << v;
        return;
    }

    if (v == std::floor(v)) {
        out << std::fixed << std::setprecision(1) << v;
    } else {
        out.unsetf(std::ios::floatfield);
        out << std::setprecision(17) << v;
    }
}

std::string extract_string(const uint8_t* data, size_t len) {
    std::string result;
    for (size_t i = 0; i < len; ++i) {
        if (data[i] == 0) break;
        if (data[i] >= 32 && data[i] <= 126) {
            result += static_cast<char>(data[i]);
        }
    }
    return result;
}

std::string extract_instrument_id(const uint8_t* data, size_t len) {
    if (!data || len == 0) return std::string();

    size_t i = 0;
    while (i < len && data[i] != 0) ++i;
    if (i >= len) return std::string();
    ++i;

    std::string result;
    while (i < len) {
        if (i + 2 < len && data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 0) break;
        if (data[i] >= 32 && data[i] <= 126) {
            result += static_cast<char>(data[i]);
        }
        ++i;
    }
    return result;
}

void parse_update_time(const uint8_t* data, size_t len, int& update_time, int& update_millisec) {
    std::string str = extract_string(data, len);

    size_t pos = str.find(' ');
    if (pos != std::string::npos) {
        str = str.substr(pos + 1);
    }

    update_time = 0;
    update_millisec = 0;

    size_t dot_pos = str.find('.');
    std::string time_part;
    std::string milli_part;

    if (dot_pos != std::string::npos) {
        time_part = str.substr(0, dot_pos);
        milli_part = str.substr(dot_pos + 1);
    } else {
        time_part = str;
    }

    std::string time_num;
    for (char c : time_part) {
        if (c != ':') {
            time_num += c;
        }
    }

    if (!time_num.empty()) {
        update_time = std::atoi(time_num.c_str());
    }

    if (!milli_part.empty()) {
        if (milli_part.length() >= 3) {
            update_millisec = std::atoi(milli_part.substr(0, 3).c_str());
        } else {
            update_millisec = std::atoi(milli_part.c_str());
        }
    }
}

void write_direct(std::ostream& out, char direct) {
    const unsigned char c = static_cast<unsigned char>(direct);
    if (c >= 32 && c <= 126) {
        out << direct;
    } else {
        out << "0";
    }
}

}  // namespace

void print_t31_csv_header(std::ostream& out, int max_price_num) {
    out << "tradingDay,instrumentId,actionDay,"
        << "lastPrice,highPrice,lowPrice,"
        << "lastVolume,volume,turnover,"
        << "preOpenInterest,openInterest,"
        << "settlementPrice,upperLimitPrice,lowerLimitPrice,preSettlementPrice,preClosePrice,bidPrice,"
        << "bidQty,bidImplyQty,"
        << "askPrice,"
        << "askQty,askImplyQty,"
        << "avgPrice,"
        << "openPrice,closePrice,"
        << "updateTime,updateMillisec,priceNum";

    for (int i = 1; i <= max_price_num; ++i) {
        out << ",price" << i << ",value" << i << "A,value" << i << "B,direct" << i;
    }
    out << "\n";
}

void print_t31_csv_row(const types::T31& q, std::ostream& out) {
    const std::string trading_day = extract_string(q.off_8_c80.data(), q.off_8_c80.size());
    const std::string instrument_id = extract_instrument_id(q.off_8_c80.data(), q.off_8_c80.size());
    const std::string action_day = extract_string(q.off_8_c80.data(), q.off_8_c80.size());

    out << trading_day << ",";
    out << instrument_id << ",";
    out << action_day << ",";

    write_float(out, q.off_88_d3[0]); out << ",";
    write_float(out, q.off_88_d3[1]); out << ",";
    write_float(out, q.off_88_d3[2]); out << ",";

    out << q.off_112_i2[0] << ",";
    out << q.off_112_i2[1] << ",";

    write_float(out, q.off_120_d1); out << ",";

    out << q.off_128_i3[0] << ",";
    out << q.off_128_i3[1] << ",";

    write_float(out, q.off_140_d8[0]); out << ",";
    write_float(out, q.off_140_d8[3]); out << ",";
    write_float(out, q.off_140_d8[4]); out << ",";
    write_float(out, q.off_140_d8[5]); out << ",";
    write_float(out, q.off_140_d8[6]); out << ",";
    write_float(out, q.off_140_d8[7]); out << ",";

    out << q.off_204_i2[0] << ",";
    out << q.off_204_i2[1] << ",";

    write_float(out, q.off_212_d1); out << ",";

    out << q.off_220_i2[0] << ",";
    out << q.off_220_i2[1] << ",";

    write_float(out, q.off_228_d1); out << ",";

    write_float(out, q.off_252_d2[0]); out << ",";
    write_float(out, q.off_252_d2[1]); out << ",";

    int update_time = 0;
    int update_millisec = 0;
    parse_update_time(q.off_236_c16.data(), q.off_236_c16.size(), update_time, update_millisec);
    out << update_time << "," << update_millisec << ",";

    int price_num = q.off_268_i1;
    out << price_num;

    int max_blocks = static_cast<int>(q.off_272_blocks.size());
    int actual_price_num = q.off_268_i1;
    if (actual_price_num > max_blocks) actual_price_num = max_blocks;

    for (int i = 0; i < 10; ++i) {
        if (i < actual_price_num) {
            const auto& block = q.off_272_blocks[i];
            out << ",";
            write_float(out, block.d0);
            out << "," << block.i[0];
            out << "," << block.i[1];
            out << "," << block.i[2];
        } else {
            out << ",0.0,0,0,0";
        }
    }

    out << "\n";
}

void print_t32_csv_header(std::ostream& out) {
    out << "tradingDay,instrumentId,lastPrice,highPrice,lowPrice,"
        << "upperLimitPrice,lowerLimitPrice,bidPrice,bidQty,askPrice,askQty,"
        << "updateTime,updateMillisec,priceNum,sequenceNo";

    for (int i = 1; i <= 10; ++i) {
        out << ",price" << i << ",volume" << i << ",implyVolume" << i << ",direct" << i;
    }
    out << "\n";
}

void print_t32_csv_row(const types::T32& q, std::ostream& out) {
    const std::string trading_day = extract_string(reinterpret_cast<const uint8_t*>(q.tradingDay), sizeof(q.tradingDay));
    const std::string instrument_id = extract_string(reinterpret_cast<const uint8_t*>(q.instrumentId), sizeof(q.instrumentId));

    int update_time = 0;
    int update_millisec = 0;
    parse_update_time(reinterpret_cast<const uint8_t*>(q.genTime), sizeof(q.genTime), update_time, update_millisec);

    out << trading_day << "," << instrument_id << ",";
    write_float(out, q.lastPrice); out << ",";
    write_float(out, q.highPrice); out << ",";
    write_float(out, q.lowPrice); out << ",";
    write_float(out, q.upperLimitPrice); out << ",";
    write_float(out, q.lowerLimitPrice); out << ",";
    write_float(out, q.bidPrice); out << ",";
    out << q.bidQty << ",";
    write_float(out, q.askPrice); out << ",";
    out << q.askQty << ",";
    out << update_time << "," << update_millisec << ",";
    out << q.priceNum << "," << q.sequenceNo;

    for (int i = 0; i < 5; ++i) {
        out << ",";
        write_float(out, q.bid[i].price);
        out << "," << q.bid[i].volume
            << "," << q.bid[i].implyVolume
            << ",";
        write_direct(out, q.bid[i].direct);
    }
    for (int i = 0; i < 5; ++i) {
        out << ",";
        write_float(out, q.ask[i].price);
        out << "," << q.ask[i].volume
            << "," << q.ask[i].implyVolume
            << ",";
        write_direct(out, q.ask[i].direct);
    }
    out << "\n";
}

}  // namespace printer
