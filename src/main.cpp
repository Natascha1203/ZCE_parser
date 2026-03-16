#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include "parser/parser.h"
#include "t_printer.h"


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



// 写入 CSV 头部（按偏移量排序）
static void write_t31_csv_header(std::ostream& out, int max_price_num) {
    printer::print_t31_csv_header(out, max_price_num);
    return;

}

// 将 T31 数据写入 CSV 行（按偏移量排序）
static void write_t31_csv_row(std::ostream& out, const types::T31& q) {
    printer::print_t31_csv_row(q, out);
    return;

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
    write_t31_csv_header(ofs, 10);
    write_t31_csv_row(ofs, q);
    
    std::cout << "CSV output written to: " << output_file << "\n";
    return 0;
}

static bool parse_types_option(const std::string& spec, batch::ParseTypeOptions* out) {
    if (!out) return false;
    batch::ParseTypeOptions opts;
    opts.enable_t31 = false;
    opts.enable_t32 = false;

    size_t start = 0;
    while (start <= spec.size()) {
        size_t end = spec.find(',', start);
        if (end == std::string::npos) end = spec.size();
        std::string token = spec.substr(start, end - start);
        for (size_t i = 0; i < token.size(); ++i) {
            if (token[i] >= 'A' && token[i] <= 'Z') {
                token[i] = static_cast<char>(token[i] - 'A' + 'a');
            }
        }
        if (token == "t31") opts.enable_t31 = true;
        else if (token == "t32") opts.enable_t32 = true;
        else if (!token.empty()) return false;
        if (end == spec.size()) break;
        start = end + 1;
    }

    if (!opts.enable_t31 && !opts.enable_t32) return false;
    *out = opts;
    return true;
}

// 自动检测模式：检测并解析所有 T31 数据包
static int auto_mode(const char* input_file, const char* output_file, const batch::ParseTypeOptions& options) {
    std::vector<uint8_t> buf;
    if (!read_file_all(input_file, buf)) {
        std::cerr << "Failed to read file: " << input_file << "\n";
        return 1;
    }

    std::cout << "File size: " << buf.size() << " bytes\n";
    std::cout << "========================================\n\n";

    // 批量解析
    if (!output_file) {
        std::cerr << "Error: Output file is required for CSV mode\n";
        return 3;
    }
    const batch::BatchCsvStats stats = batch::batchParseToCsv(buf.data(), buf.size(), output_file, options);

    if (stats.detected == 0) {
        std::cerr << "No packets detected.\n";
        return 2;
    }

    std::cout << "\n========================================\n";
    std::cout << "Detected: " << stats.detected
              << ", parsed_ok: " << stats.parsed_ok
              << ", parsed_fail: " << stats.parsed_fail << "\n";

    #if 0
    std::ofstream ofs(output_file);
    if (!ofs) {
        std::cerr << "Failed to open output file: " << output_file << "\n";
        return 3;
    }

    // 始终输出10档
    int max_price_num = 10;

    // 写入 CSV 头部
    write_t31_csv_header(ofs, max_price_num);

    // 写入每一行数据
    for (const auto& t31 : results) {
        write_t31_csv_row(ofs, t31);
    }

    std::cout << "CSV output written to: " << output_file << "\n";
    #endif
    if (stats.t31_rows > 0) {
        std::cout << "T31 rows: " << stats.t31_rows << ", csv: " << stats.t31_csv_path << "\n";
    }
    if (stats.t32_rows > 0) {
        std::cout << "T32 rows: " << stats.t32_rows << ", csv: " << stats.t32_csv_path << "\n";
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage:\n";
        std::cerr << "  Manual mode:  " << argv[0] << " <data.bin> <output.csv>\n";
        std::cerr << "  Auto mode:    " << argv[0] << " -a <data.bin> <output.csv> [--types t31,t32]\n";
        std::cerr << "\nOptions:\n";
        std::cerr << "  -a, --auto    Auto-detect mode (search for T31 packets with signature validation)\n";
        std::cerr << "\nExamples:\n";
        std::cerr << "  " << argv[0] << " data.bin output.csv       # Manual mode\n";
        std::cerr << "  " << argv[0] << " -a data.bin output.csv --types t31,t32\n";
        return 1;
    }

    // 检查是否为自动模式
    bool auto_detect = false;
    int file_arg_idx = 1;
    batch::ParseTypeOptions type_options;
    std::string type_spec = "t31,t32";

    if (std::strcmp(argv[1], "-a") == 0 || std::strcmp(argv[1], "--auto") == 0) {
        auto_detect = true;
        file_arg_idx = 2;
        
        if (argc < 4) {
            std::cerr << "Error: Missing input file or output file\n";
            return 1;
        }
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind("--types=", 0) == 0) {
            type_spec = arg.substr(8);
        } else if (arg == "--types" && i + 1 < argc) {
            type_spec = argv[i + 1];
        }
    }

    if (!parse_types_option(type_spec, &type_options)) {
        std::cerr << "Invalid --types value: " << type_spec << " (use t31,t32)\n";
        return 1;
    }

    const char* input_file = argv[file_arg_idx];
    const char* output_file = argv[file_arg_idx + 1];

    if (auto_detect) {
        return auto_mode(input_file, output_file, type_options);
    } else {
        return manual_mode(input_file, output_file);
    }
}
