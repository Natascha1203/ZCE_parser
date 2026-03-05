#include <fstream>
#include <iostream>
#include <vector>

#include "parser/parser.h"
#include "common/print.h"

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

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " data.bin\n";
        return 1;
    }

    std::vector<uint8_t> buf;
    if (!read_file_all(argv[1], buf)) {
        std::cerr << "Failed to read file: " << argv[1] << "\n";
        return 1;
    }

    types::T31 q;
    if (!parser::parseT31(buf.data(), buf.size(), &q)) {
        std::cerr << "parseT31 failed, buf_size=" << buf.size() << "\n";
        return 2;
    }

    // 简单打印几个字段示例（你可以做更完整的dump函数）
    std::cout << "off_236_c16 ascii: \""
              << common::bytes_to_ascii_printable(q.off_236_c16.data(), q.off_236_c16.size())
              << "\"\n";

    std::cout << "off_120_d1: " << q.off_120_d1 << "\n";
    std::cout << "off_668_i1: " << q.off_668_i1 << "\n";

    return 0;
}