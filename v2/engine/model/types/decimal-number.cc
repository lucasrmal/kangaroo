/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  US
 */

#include <stdexcept>

#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "model/types/decimal-number.h"

namespace kangaroo {

const double DecimalNumber::POW[DecimalNumber::kMaxDecimals + 1] = {
    1., 10., 100., 1000., 10000., 100000., 1000000.};

DecimalNumber::DecimalNumber(const int _int, int _precision)
    : m_baseDecimalNumber((uint64_t)(POW[_precision] * _int)),
      m_precision(_precision) {
  if (_precision > kMaxDecimals) {
    throw std::invalid_argument(absl::StrCat("Precision cannot be larger than ",
                                kMaxDecimals));
  } else if (_precision < 0) {
    throw std::invalid_argument("Precision has to be at least 0.");
  }
}

DecimalNumber::DecimalNumber(const double _double, int _precision)
    : m_baseDecimalNumber((uint64_t)round(_double * POW[_precision])),
      m_precision(_precision) {
  if (_precision > kMaxDecimals) {
    throw std::invalid_argument(absl::StrCat("Precision cannot be larger than ",
                                kMaxDecimals));
  } else if (_precision < 0) {
    throw std::invalid_argument("Precision has to be at least 0.");
  }
}

DecimalNumber::DecimalNumber(const std::string& p_string, int _precision)
    : m_precision(_precision) {
  double temp = 0;
  if (!absl::SimpleAtod(p_string, &temp)) {
    throw std::invalid_argument(absl::StrCat("Not a number!: ", p_string));
  }
  m_baseDecimalNumber = (uint64_t)round(temp * POW[_precision]);

  if (_precision > kMaxDecimals) {
    throw std::invalid_argument(absl::StrCat("Precision cannot be larger than ",
                                kMaxDecimals));
  } else if (_precision < 0) {
    throw std::invalid_argument("Precision has to be at least 0.");
  }
}
}  // namespace kangaroo
