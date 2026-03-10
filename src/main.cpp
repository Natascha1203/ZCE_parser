#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>

#include "parser/parser.h"
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

// 写入 CSV 头部
static void write_csv_header(std::ostream& out, int max_price_num) {
    out << "tradingDay,instrumentId,preClosePrice,preSettlementPrice,lastPrice,"
        << "volume,turnover,preOpenInterest,openInterest,openPrice,"
        << "highPrice,lowPrice,upperLimitPrice,lowerLimitPrice,closePrice,"
        << "settlementPrice,actionDay,updateTime,updateMillisec,priceNum";
    
    // 动态添加 price 列
    for (int i = 1; i <= max_price_num; ++i) {
        out << ",price" << i << ",value" << i << "A,value" << i << "B,direct" << i;
    }
    out << "\n";
}

// 将 T31 数据写入 CSV 行
static void write_csv_row(std::ostream& out, const types::T31& q) {
    out << std::setprecision(17);
    
    // tradingDay (从 off_8_c80 提取)
    out << extract_string(q.off_8_c80.data(), q.off_8_c80.size()) << ",";
    
    // instrumentId (从 off_8_c80 提取，这里假设和 tradingDay 相同，实际可能需要不同的偏移)
    out << extract_string(q.off_8_c80.data(), q.off_8_c80.size()) << ",";
    
    // preClosePrice
    out << q.off_140_d8[6] << ",";
    
    // preSettlementPrice
    out << q.off_140_d8[5] << ",";
    
    // lastPrice
    out << q.off_140_d8[7] << ",";
    
    // volume
    out << q.off_128_i3[2] << ",";
    
    // turnover
    out << q.off_120_d1 << ",";
    
    // preOpenInterest
    out << q.off_128_i3[0] << ",";
    
    // openInterest
    out << q.off_128_i3[1] << ",";
    
    // openPrice
    out << q.off_252_d2[0] << ",";
    
    // highPrice
    out << q.off_88_d3[1] << ",";
    
    // lowPrice
    out << q.off_88_d3[2] << ",";
    
    // upperLimitPrice
    out << q.off_140_d8[3] << ",";
    
    // lowerLimitPrice
    out << q.off_140_d8[4] << ",";
    
    // closePrice
    out << q.off_140_d8[0] << ",";
    
    // settlementPrice
    out << q.off_252_d2[1] << ",";
    
    // actionDay (从 off_8_c80 提取)
    out << extract_string(q.off_8_c80.data(), q.off_8_c80.size()) << ",";
    
    // updateTime (从 off_236_c16 提取)
    out << extract_string(q.off_236_c16.data(), q.off_236_c16.size()) << ",";
    
    // updateMillisec (从 off_236_c16 提取)
    out << extract_string(q.off_236_c16.data(), q.off_236_c16.size()) << ",";
    
    // priceNum
    int price_num = q.off_268_i1;
    out << price_num;
    
    // 动态输出 price 数据
    int max_blocks = static_cast<int>(q.off_272_blocks.size());
    if (price_num > max_blocks) price_num = max_blocks;
    
    for (int i = 0; i < price_num; ++i) {
        const auto& block = q.off_272_blocks[i];
        out << "," << block.d0;           // priceX
        out << "," << block.i[0];         // valueXA
        out << "," << block.i[1];         // valueXB
        out << "," << block.i[2];         // directX
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

    // 写入 CSV
    int max_price_num = q.off_268_i1;
    if (max_price_num > 10) max_price_num = 10;
    
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

    // 找出最大的 priceNum
    int max_price_num = 0;
    for (const auto& res : results) {
        if (res.success) {
            int pn = res.t31_data.off_268_i1;
            if (pn > max_price_num) max_price_num = pn;
        }
    }
    if (max_price_num > 10) max_price_num = 10;

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
