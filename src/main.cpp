#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <cstdio>

#include <cmath>
#include "parser/parser.h"

// 写入浮点数：整数显示为 x.0，非整数显示高精度
static void write_float(std::ostream& out, double v) {
    const double abs_v = std::fabs(v);

    // Very large values (e.g. DBL_MAX placeholders) are more readable in scientific notation.
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

static std::string extract_string(const uint8_t* data, size_t len);

// 解析时间字符串 "10:51:52.147" 或 "09:51:21"
// 输出: updateTime = 105152, updateMillisec = 147
// 如果没有毫秒部分，updateMillisec 为 0
static void parse_update_time(const uint8_t* data, size_t len, int& updateTime, int& updateMillisec) {
    std::string str = extract_string(data, len);

    // 去掉空格
    size_t pos = str.find(' ');
    if (pos != std::string::npos) {
        str = str.substr(pos + 1);
    }

    updateTime = 0;
    updateMillisec = 0;

    // 找小数点位置
    size_t dot_pos = str.find('.');
    std::string time_part;
    std::string milli_part;

    if (dot_pos != std::string::npos) {
        time_part = str.substr(0, dot_pos);
        milli_part = str.substr(dot_pos + 1);
    } else {
        time_part = str;
    }

    // 去掉冒号，拼接成数字
    std::string time_num;
    for (char c : time_part) {
        if (c != ':') {
            time_num += c;
        }
    }

    if (!time_num.empty()) {
        updateTime = std::atoi(time_num.c_str());
    }

    // 毫秒部分
    if (!milli_part.empty()) {
        // 取前3位作为毫秒
        if (milli_part.length() >= 3) {
            updateMillisec = std::atoi(milli_part.substr(0, 3).c_str());
        } else {
            updateMillisec = std::atoi(milli_part.c_str());
        }
    }
}
#include "common/print.h"
#include "detector/packet_detector.h"
#include "batch/batch_parser.h"

static bool read_file_all(const char* path, std::vector<uint8_t>& out) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return false;
    ifs.seekg(0, std::ios::end);
    std::streamsize n = ifs.tellg();
    if (n < 0) return false;
    ifs.seekg(0, std::ios::beg);
    out.resize(static_cast<size_t>(n));
    return static_cast<bool>(ifs.read(reinterpret_cast<char*>(out.data()), n));
}

// 从字节数组中提取字符串（去除尾部的0和不可打印字符）
static std::string extract_string(const uint8_t* data, size_t len) {
    std::string result;
    for (size_t i = 0; i < len; ++i) {
        if (data[i] == 0) break;
        if (data[i] >= 32 && data[i] <= 126) {
            result += static_cast<char>(data[i]);
        }
    }
    return result;
}

static std::string extract_instrument_id(const uint8_t* data, size_t len) {
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

// 写入 CSV 头部（按偏移量排序）
static void write_csv_header(std::ostream& out, int max_price_num) {
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

    // 动态添加 price 列
    for (int i = 1; i <= max_price_num; ++i) {
        out << ",price" << i << ",value" << i << "A,value" << i << "B,direct" << i;
    }
    out << "\n";
}

// 将 T31 数据写入 CSV 行（按偏移量排序）
static void write_csv_row(std::ostream& out, const types::T31& q) {
    const std::string trading_day = extract_string(q.off_8_c80.data(), q.off_8_c80.size());
    const std::string instrument_id = extract_instrument_id(q.off_8_c80.data(), q.off_8_c80.size());
    const std::string action_day = extract_string(q.off_8_c80.data(), q.off_8_c80.size());

    // off_8_c80
    out << trading_day << ",";
    out << instrument_id << ",";
    out << action_day << ",";

    // off_88_d3: lastPrice, highPrice, lowPrice
    write_float(out, q.off_88_d3[0]); out << ",";
    write_float(out, q.off_88_d3[1]); out << ",";
    write_float(out, q.off_88_d3[2]); out << ",";

    // off_112_i2: lastVolume, volume
    out << q.off_112_i2[0] << ",";
    out << q.off_112_i2[1] << ",";

    // off_120_d1: turnover
    write_float(out, q.off_120_d1); out << ",";

    // off_128_i3: preOpenInterest, openInterest
    out << q.off_128_i3[0] << ",";
    out << q.off_128_i3[1] << ",";

    // off_140_d8: settlementPrice, upperLimitPrice, lowerLimitPrice, preSettlementPrice, preClosePrice, bidPrice
    write_float(out, q.off_140_d8[0]); out << ",";
    write_float(out, q.off_140_d8[3]); out << ",";
    write_float(out, q.off_140_d8[4]); out << ",";
    write_float(out, q.off_140_d8[5]); out << ",";
    write_float(out, q.off_140_d8[6]); out << ",";
    write_float(out, q.off_140_d8[7]); out << ",";

    // off_204_i2: bidQty, bidImplyQty
    out << q.off_204_i2[0] << ",";
    out << q.off_204_i2[1] << ",";

    // off_212_d1: askPrice
    write_float(out, q.off_212_d1); out << ",";

    // off_220_i2: askQty, askImplyQty
    out << q.off_220_i2[0] << ",";
    out << q.off_220_i2[1] << ",";

    // off_228_d1: avgPrice
    write_float(out, q.off_228_d1); out << ",";

    // off_252_d2: openPrice, closePrice
    write_float(out, q.off_252_d2[0]); out << ",";
    write_float(out, q.off_252_d2[1]); out << ",";

    // off_236_c16: updateTime, updateMillisec
    int updateTime = 0;
    int updateMillisec = 0;
    parse_update_time(q.off_236_c16.data(), q.off_236_c16.size(), updateTime, updateMillisec);
    out << updateTime << "," << updateMillisec << ",";

    // off_268_i1: priceNum
    int price_num = q.off_268_i1;
    out << price_num;

    // off_272_blocks: price1~10, value1A~10B, direct1~10
    int max_blocks = static_cast<int>(q.off_272_blocks.size());
    int actual_price_num = q.off_268_i1;
    if (actual_price_num > max_blocks) actual_price_num = max_blocks;

    for (int i = 0; i < 10; ++i) {
        if (i < actual_price_num) {
            const auto& block = q.off_272_blocks[i];
            out << ",";
            write_float(out, block.d0);        // priceX
            out << "," << block.i[0];         // valueXA
            out << "," << block.i[1];         // valueXB
            out << "," << block.i[2];         // directX
        } else {
            // 不存在则填默认值
            out << ",0.0,0,0,0";
        }
    }

    out << "\n";
}

// 手动模式：直接解析整个文件为单个 T31
static int manual_mode(const char* input_file, const char* output_file) {
    std::vector<uint8_t> buf;
    if (!read_file_all(input_file, buf)) {
        std::cerr << "Failed to read file: " << input_file << "\n";
        return 1;
    }

    types::T31 q;
    if (!parser::parseT31(buf.data(), buf.size(), &q)) {
        std::cerr << "parseT31 failed, buf_size=" << buf.size() << "\n";
        return 2;
    }

    if (!output_file) {
        std::cerr << "Error: Output file is required for CSV mode\n";
        return 3;
    }

    std::ofstream ofs(output_file);
    if (!ofs) {
        std::cerr << "Failed to open output file: " << output_file << "\n";
        return 3;
    }

    // 写入 CSV，始终输出10档
    int max_price_num = 10;

    write_csv_header(ofs, max_price_num);
    write_csv_row(ofs, q);
    
    std::cout << "CSV output written to: " << output_file << "\n";
    return 0;
}

// 自动检测模式：检测并解析所有 T31 数据包
static int auto_mode(const char* input_file, const char* output_file) {
    std::vector<uint8_t> buf;
    if (!read_file_all(input_file, buf)) {
        std::cerr << "Failed to read file: " << input_file << "\n";
        return 1;
    }

    std::cout << "File size: " << buf.size() << " bytes\n";
    std::cout << "========================================\n\n";

    // 批量解析
    auto results = batch::batchParseT31(buf.data(), buf.size());

    if (results.empty()) {
        std::cerr << "No packets parsed.\n";
        return 2;
    }

    std::cout << "\n========================================\n";
    std::cout << "Total: " << results.size() << " packet(s) parsed successfully\n\n";

    if (!output_file) {
        std::cerr << "Error: Output file is required for CSV mode\n";
        return 3;
    }

    std::ofstream ofs(output_file);
    if (!ofs) {
        std::cerr << "Failed to open output file: " << output_file << "\n";
        return 3;
    }

    // 始终输出10档
    int max_price_num = 10;

    // 写入 CSV 头部
    write_csv_header(ofs, max_price_num);

    // 写入每一行数据
    for (const auto& res : results) {
        if (res.success) {
            write_csv_row(ofs, res.t31_data);
        }
    }

    std::cout << "CSV output written to: " << output_file << "\n";
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage:\n";
        std::cerr << "  Manual mode:  " << argv[0] << " <data.bin> <output.csv>\n";
        std::cerr << "  Auto mode:    " << argv[0] << " -a <data.bin> <output.csv>\n";
        std::cerr << "\nOptions:\n";
        std::cerr << "  -a, --auto    Auto-detect mode (search for T31 packets with signature validation)\n";
        std::cerr << "\nExamples:\n";
        std::cerr << "  " << argv[0] << " data.bin output.csv       # Manual mode\n";
        std::cerr << "  " << argv[0] << " -a data.bin output.csv    # Auto mode\n";
        return 1;
    }

    // 检查是否为自动模式
    bool auto_detect = false;
    int file_arg_idx = 1;

    if (std::strcmp(argv[1], "-a") == 0 || std::strcmp(argv[1], "--auto") == 0) {
        auto_detect = true;
        file_arg_idx = 2;
        
        if (argc < 4) {
            std::cerr << "Error: Missing input file or output file\n";
            return 1;
        }
    }

    const char* input_file = argv[file_arg_idx];
    const char* output_file = argv[file_arg_idx + 1];

    if (auto_detect) {
        return auto_mode(input_file, output_file);
    } else {
        return manual_mode(input_file, output_file);
    }
}
