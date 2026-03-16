# cppParser Architecture

## Overview
`cppParser` parses market binary streams into typed messages and exports CSV.

Current supported message types:
- `T31`
- `T32`
- `T36` (parser exists, currently not exported by batch CSV flow)

The executable is built from:
- `src/main.cpp`
- `src/parser.cpp`
- `src/t31_parser.cpp`
- `src/t32_parser.cpp`
- `src/t36_parser.cpp`
- `src/detector/packet_detector.cpp`
- `src/batch/batch_parser.cpp`
- `src/t_printer.cpp`

## Module Layers

### 1) Detector layer
Files:
- `include/detector/packet_detector.h`
- `src/detector/packet_detector.cpp`

Responsibilities:
- Scan raw bytes and detect packet boundaries/signatures.
- Return `PacketInfo { offset, type, length }`.
- Provide:
  - `detectT31Packets(...)` (legacy/specific)
  - `detectPackets(...)` (multi-type detection)
  - `validateT31Packet(...)`
  - `validateT32Packet(...)`

### 2) Parser layer
Files:
- `include/parser/parser.h`
- `src/parser.cpp`
- `src/t31_parser.cpp`
- `src/t32_parser.cpp`
- `src/t36_parser.cpp`

Responsibilities:
- Parse one packet into one typed struct.
- Type-specific parsers:
  - `parseT31(...)`
  - `parseT32(...)` (direct `memcpy` for packed little-endian struct)
  - `parseT36(...)`
- Unified dispatch:
  - `parseByType(MsgType, ...)`

### 3) Printer layer
Files:
- `include/t_printer.h`
- `src/t_printer.cpp`

Responsibilities:
- Convert typed structs to CSV text.
- T31:
  - `print_t31_csv_header(...)`
  - `print_t31_csv_row(...)`
- T32:
  - `print_t32_csv_header(...)`
  - `print_t32_csv_row(...)`

### 4) Batch layer
Files:
- `include/batch/batch_parser.h`
- `src/batch/batch_parser.cpp`

Responsibilities:
- End-to-end batch processing over a raw buffer.
- Main API:
  - `batchParseToCsv(data, len, output_base_csv, options)`
- Dynamic parse type control:
  - `ParseTypeOptions { enable_t31, enable_t32 }`
- Output split by type:
  - `<base>_t31.csv`
  - `<base>_t32.csv`
- Stats:
  - `BatchCsvStats { detected, parsed_ok, parsed_fail, t31_rows, t32_rows, ... }`

## Runtime Flow

1. `main.cpp` reads full input file into memory.
2. In auto mode, `--types` is parsed into `ParseTypeOptions`.
3. `batchParseToCsv(...)`:
   - calls `detectPackets(...)`
   - for each packet:
     - dispatch to `parseT31` / `parseT32`
     - writes row via matching printer function
4. Program prints summary and generated CSV paths.

## CLI (current)

- Manual mode (single T31 parse):
```bash
cppParser <data.bin> <output.csv>
```

- Auto mode (multi-packet detect + export):
```bash
cppParser -a <data.bin> <output.csv> [--types t31,t32]
```

Examples:
```bash
cppParser -a data.bin out.csv --types t31
cppParser -a data.bin out.csv --types t32
cppParser -a data.bin out.csv --types t31,t32
```

## Key Headers / Types

- `types::T31` -> `include/types/t31.h`
- `types::T32` -> `include/types/t32.h`
- `parser::MsgType` -> `include/parser/parser.h`
- `detector::PacketInfo` -> `include/detector/packet_detector.h`

## Notes

- Project is configured with C++11 in `CMakeLists.txt`.
- Struct packing/endian assumptions are important for binary parse correctness.
