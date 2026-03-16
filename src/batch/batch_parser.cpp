#include "batch/batch_parser.h"
#include "detector/packet_detector.h"
#include "t_printer.h"

#include <fstream>
#include <iostream>
#include <string>

namespace batch {



static std::string remove_csv_ext(const std::string& path) {
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".csv") {
        return path.substr(0, path.size() - 4);
    }
    return path;
}

BatchCsvStats batchParseToCsv(
    const uint8_t* data,
    size_t len,
    const std::string& output_base_csv,
    const ParseTypeOptions& options) {
    BatchCsvStats stats;
    if (!data || output_base_csv.empty()) {
        return stats;
    }

    const std::vector<detector::PacketInfo> packets = detector::detectPackets(data, len);
    for (size_t i = 0; i < packets.size(); ++i) {
        if ((packets[i].type == parser::MsgType::T31 && options.enable_t31) ||
            (packets[i].type == parser::MsgType::T32 && options.enable_t32)) {
            ++stats.detected;
        }
    }
    if (packets.empty()) {
        return stats;
    }

    const std::string base = remove_csv_ext(output_base_csv);
    stats.t31_csv_path = base + "_t31.csv";
    stats.t32_csv_path = base + "_t32.csv";

    std::ofstream t31_ofs;
    std::ofstream t32_ofs;
    bool t31_header_written = false;
    bool t32_header_written = false;

    for (size_t i = 0; i < packets.size(); ++i) {
        const detector::PacketInfo& pkt = packets[i];
        if (pkt.type == parser::MsgType::T31 && options.enable_t31) {
            types::T31 t31{};
            if (parser::parseT31(data + pkt.offset, pkt.length, &t31)) {
                if (!t31_header_written) {
                    t31_ofs.open(stats.t31_csv_path.c_str(), std::ios::out | std::ios::trunc);
                    if (!t31_ofs) {
                        ++stats.parsed_fail;
                        continue;
                    }
                    printer::print_t31_csv_header(t31_ofs, 10);
                    t31_header_written = true;
                }
                printer::print_t31_csv_row(t31, t31_ofs);
                ++stats.parsed_ok;
                ++stats.t31_rows;
            } else {
                ++stats.parsed_fail;
            }
        } else if (pkt.type == parser::MsgType::T32 && options.enable_t32) {
            types::T32 t32{};
            if (parser::parseT32(data + pkt.offset, pkt.length, &t32)) {
                if (!t32_header_written) {
                    t32_ofs.open(stats.t32_csv_path.c_str(), std::ios::out | std::ios::trunc);
                    if (!t32_ofs) {
                        ++stats.parsed_fail;
                        continue;
                    }
                    printer::print_t32_csv_header(t32_ofs);
                    t32_header_written = true;
                }
                printer::print_t32_csv_row(t32, t32_ofs);
                ++stats.parsed_ok;
                ++stats.t32_rows;
            } else {
                ++stats.parsed_fail;
            }
        }
    }

    return stats;
}

} // namespace batch
