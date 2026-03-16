#pragma once

#include <iosfwd>

#include "types/t31.h"

namespace printer {

void write_t31_csv_header(std::ostream& out, int max_price_num);
void print_t31(const types::T31& q, std::ostream& out);

}  // namespace printer

