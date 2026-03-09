#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>

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

static void print_t31(const types::T31& q, std::ostream& out) {
    out << "===== T31 Parse Result =====\n\n";

    // 8: char[80]
    out << "off_8_c80 (char[80]):\n  \"" 
        << common::bytes_to_ascii_printable(q.off_8_c80.data(), q.off_8_c80.size()) 
        << "\"\n\n";

    // 88: double[3]
    out << "off_88_d3 (double[3]):\n  ";
    for (size_t i = 0; i < q.off_88_d3.size(); ++i) {
        out << q.off_88_d3[i];
        if (i < q.off_88_d3.size() - 1) out << ", ";
    }
    out << "\n\n";

    // 112: int[2]
    out << "off_112_i2 (int32[2]):\n  ";
    for (size_t i = 0; i < q.off_112_i2.size(); ++i) {
        out << q.off_112_i2[i];
        if (i < q.off_112_i2.size() - 1) out << ", ";
    }
    out << "\n\n";

    // 120: double[1]
    out << "off_120_d1 (double):\n  " << q.off_120_d1 << "\n\n";

    // 128: int[3]
    out << "off_128_i3 (int32[3]):\n  ";
    for (size_t i = 0; i < q.off_128_i3.size(); ++i) {
        out << q.off_128_i3[i];
        if (i < q.off_128_i3.size() - 1) out << ", ";
    }
    out << "\n\n";

    // 140: double[8]
    out << "off_140_d8 (double[8]):\n  ";
    for (size_t i = 0; i < q.off_140_d8.size(); ++i) {
        out << q.off_140_d8[i];
        if (i < q.off_140_d8.size() - 1) out << ", ";
    }
    out << "\n\n";

    // 204: int[2]
    out << "off_204_i2 (int32[2]):\n  ";
    for (size_t i = 0; i < q.off_204_i2.size(); ++i) {
        out << q.off_204_i2[i];
        if (i < q.off_204_i2.size() - 1) out << ", ";
    }
    out << "\n\n";

    // 212: double[1]
    out << "off_212_d1 (double):\n  " << q.off_212_d1 << "\n\n";

    // 220: int[2]
    out << "off_220_i2 (int32[2]):\n  ";
    for (size_t i = 0; i < q.off_220_i2.size(); ++i) {
        out << q.off_220_i2[i];
        if (i < q.off_220_i2.size() - 1) out << ", ";
    }
    out << "\n\n";

    // 228: double[1]
    out << "off_228_d1 (double):\n  " << q.off_228_d1 << "\n\n";

    // 236: char[16]
    out << "off_236_c16 (char[16]):\n  \"" 
        << common::bytes_to_ascii_printable(q.off_236_c16.data(), q.off_236_c16.size()) 
        << "\"\n\n";

    // 252: double[2]
    out << "off_252_d2 (double[2]):\n  ";
    for (size_t i = 0; i < q.off_252_d2.size(); ++i) {
        out << q.off_252_d2[i];
        if (i < q.off_252_d2.size() - 1) out << ", ";
    }
    out << "\n\n";

    // 268: int[1]
    out << "off_268_i1 (int32):\n  " << q.off_268_i1 << "\n\n";

    // 272: 10 blocks
    out << "off_272_blocks (Block32[10]):\n";
    for (size_t k = 0; k < q.off_272_blocks.size(); ++k) {
        const auto& b = q.off_272_blocks[k];
        out << "  [" << k << "] d0=" << b.d0
            << ", i=[" << b.i[0] << ", " << b.i[1] << ", " << b.i[2] << "]"
            << ", c=\"" << common::bytes_to_ascii_printable(b.c.data(), b.c.size()) << "\"\n";
    }
    out << "\n";

    // 592: char[36]
    out << "off_592_c36 (char[36]):\n  \"" 
        << common::bytes_to_ascii_printable(q.off_592_c36.data(), q.off_592_c36.size()) 
        << "\"\n\n";

    // 628: double[5]
    out << "off_628_d5 (double[5]):\n  ";
    for (size_t i = 0; i < q.off_628_d5.size(); ++i) {
        out << q.off_628_d5[i];
        if (i < q.off_628_d5.size() - 1) out << ", ";
    }
    out << "\n\n";

    // 668: int[1]
    out << "off_668_i1 (int32):\n  " << q.off_668_i1 << "\n\n";

    // 672: int8[4]
    out << "off_672_i8_4 (int8[4]):\n  ";
    for (size_t i = 0; i < q.off_672_i8_4.size(); ++i) {
        out << static_cast<int>(q.off_672_i8_4[i]);
        if (i < q.off_672_i8_4.size() - 1) out << ", ";
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

    // 输出到文件或控制台
    if (output_file) {
        std::ofstream ofs(output_file);
        if (!ofs) {
            std::cerr << "Failed to open output file: " << output_file << "\n";
            return 3;
        }
        ofs << std::setprecision(17);
        print_t31(q, ofs);
        std::cout << "Output written to: " << output_file << "\n";
    } else {
        std::cout << std::setprecision(17);
        print_t31(q, std::cout);
    }

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

    // 输出结果
    if (output_file) {
        std::ofstream ofs(output_file);
        if (!ofs) {
            std::cerr << "Failed to open output file: " << output_file << "\n";
            return 3;
        }
        ofs << std::setprecision(17);
        
        for (size_t i = 0; i < results.size(); ++i) {
            const auto& res = results[i];
            if (res.success) {
                ofs << "========================================\n";
                ofs << "Packet #" << (i + 1) << " at offset " << res.offset << "\n";
                ofs << "========================================\n\n";
                print_t31(res.t31_data, ofs);
                ofs << "\n\n";
            }
        }
        std::cout << "Output written to: " << output_file << "\n";
    } else {
        std::cout << std::setprecision(17);
        for (size_t i = 0; i < results.size(); ++i) {
            const auto& res = results[i];
            if (res.success) {
                std::cout << "========================================\n";
                std::cout << "Packet #" << (i + 1) << " at offset " << res.offset << "\n";
                std::cout << "========================================\n\n";
                print_t31(res.t31_data, std::cout);
                std::cout << "\n\n";
            }
        }
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage:\n";
        std::cerr << "  Manual mode:  " << argv[0] << " <data.bin> [output.txt]\n";
        std::cerr << "  Auto mode:    " << argv[0] << " -a <data.bin> [output.txt]\n";
        std::cerr << "\nOptions:\n";
        std::cerr << "  -a, --auto    Auto-detect mode (search for T31 packets with signature validation)\n";
        std::cerr << "\nExamples:\n";
        std::cerr << "  " << argv[0] << " data.bin              # Manual mode, output to console\n";
        std::cerr << "  " << argv[0] << " data.bin out.txt      # Manual mode, output to file\n";
        std::cerr << "  " << argv[0] << " -a data.bin           # Auto mode, output to console\n";
        std::cerr << "  " << argv[0] << " -a data.bin out.txt   # Auto mode, output to file\n";
        return 1;
    }

    // 检查是否为自动模式
    bool auto_detect = false;
    int file_arg_idx = 1;

    if (std::strcmp(argv[1], "-a") == 0 || std::strcmp(argv[1], "--auto") == 0) {
        auto_detect = true;
        file_arg_idx = 2;
        
        if (argc < 3) {
            std::cerr << "Error: Missing input file\n";
            return 1;
        }
    }

    const char* input_file = argv[file_arg_idx];
    const char* output_file = (argc > file_arg_idx + 1) ? argv[file_arg_idx + 1] : nullptr;

    if (auto_detect) {
        return auto_mode(input_file, output_file);
    } else {
        return manual_mode(input_file, output_file);
    }
}
