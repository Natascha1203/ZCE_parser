#pragma once

#include <iosfwd>

#include "types/t31.h"
#include "types/t32.h"

namespace printer {

void print_t31_csv_header(std::ostream& out, int max_price_num);
void print_t31_csv_row(const types::T31& q, std::ostream& out);
void print_t32_csv_header(std::ostream& out);
void print_t32_csv_row(const types::T32& q, std::ostream& out);

}  // namespace printer
