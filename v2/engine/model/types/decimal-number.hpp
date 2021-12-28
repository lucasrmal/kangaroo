/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2008-2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
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

#ifndef ENGINE_MODEL_DECIMAL_NUMBER_HPP
#define ENGINE_MODEL_DECIMAL_NUMBER_HPP

#include <cmath>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"

#define POW_10(a) POW[a]

namespace kangaroo {

inline DecimalNumber DecimalNumber::inverse() const {
  DecimalNumber a = *this;
  a.m_baseDecimalNumber =
      (uint64_t)round(POW_10(a.m_precision) / (double)a.m_baseDecimalNumber);
  return a;
}

inline void DecimalNumber::clear() { m_baseDecimalNumber = 0; }

inline DecimalNumber DecimalNumber::abs() const {
  DecimalNumber a = *this;
  a.m_baseDecimalNumber = std::llabs(a.m_baseDecimalNumber);
  return a;
}

inline DecimalNumber DecimalNumber::operator+(
    const DecimalNumber& p_amount) const {
  return DecimalNumber(*this) += p_amount;
}

inline DecimalNumber DecimalNumber::operator-(
    const DecimalNumber& p_amount) const {
  return DecimalNumber(*this) -= p_amount;
}

inline DecimalNumber DecimalNumber::operator-() const {
  DecimalNumber a = *this;
  a.m_baseDecimalNumber = -a.m_baseDecimalNumber;
  return a;
}

inline DecimalNumber DecimalNumber::operator*(
    const DecimalNumber& p_amount) const {
  return DecimalNumber(*this) *= p_amount;
}

inline DecimalNumber DecimalNumber::operator/(
    const DecimalNumber& p_amount) const {
  return DecimalNumber(*this) /= p_amount;
}

inline DecimalNumber DecimalNumber::operator+(double p_amount) const {
  return DecimalNumber(*this) += p_amount;
}

inline DecimalNumber DecimalNumber::operator-(double p_amount) const {
  return DecimalNumber(*this) -= p_amount;
}

inline DecimalNumber DecimalNumber::operator*(double p_amount) const {
  return DecimalNumber(*this) *= p_amount;
}

inline DecimalNumber DecimalNumber::operator/(double p_amount) const {
  return DecimalNumber(*this) /= p_amount;
}

inline uint64_t DecimalNumber::toSame(const DecimalNumber& _other) const {
  return (_other.m_precision == m_precision)
             ? _other.m_baseDecimalNumber
             : (uint64_t)round(pow(10.0, m_precision - _other.m_precision) *
                               (double)_other.m_baseDecimalNumber);
}

inline uint64_t DecimalNumber::toPrec(int _prec) const {
  return (m_precision == (int)_prec)
             ? m_baseDecimalNumber
             : (uint64_t)round(pow(10.0, _prec - m_precision) *
                               (double)m_baseDecimalNumber);
}

inline DecimalNumber DecimalNumber::operator+=(const DecimalNumber& p_amount) {
  if (m_precision >= p_amount.m_precision) {
    m_baseDecimalNumber += toSame(p_amount);
  } else {
    m_baseDecimalNumber =
        toPrec(p_amount.m_precision) + p_amount.m_baseDecimalNumber;
    m_precision = p_amount.m_precision;
  }

  return *this;
}

inline DecimalNumber DecimalNumber::operator-=(const DecimalNumber& p_amount) {
  if (m_precision >= p_amount.m_precision) {
    m_baseDecimalNumber -= toSame(p_amount);
  } else {
    m_baseDecimalNumber =
        toPrec(p_amount.m_precision) - p_amount.m_baseDecimalNumber;
    m_precision = p_amount.m_precision;
  }

  return *this;
}

inline DecimalNumber DecimalNumber::operator*=(const DecimalNumber& p_amount) {
  if (m_baseDecimalNumber == 0 || p_amount.m_baseDecimalNumber == 0) {
    m_baseDecimalNumber = 0;
  } else {
    if (m_precision >= p_amount.m_precision) {
      m_baseDecimalNumber *= toSame(p_amount);
      m_baseDecimalNumber /= POW_10(m_precision);
    } else {
      m_baseDecimalNumber =
          toPrec(p_amount.m_precision) * p_amount.m_baseDecimalNumber;
      m_precision = p_amount.m_precision;
      m_baseDecimalNumber /= POW_10(m_precision);
    }
  }

  return *this;
}

inline DecimalNumber DecimalNumber::operator/=(const DecimalNumber& p_amount) {
  if (m_precision >= p_amount.m_precision) {
    m_baseDecimalNumber = (uint64_t)round(
        (((double)m_baseDecimalNumber) / ((double)toSame(p_amount))) *
        POW_10(m_precision));
  } else {
    m_baseDecimalNumber =
        (uint64_t)round((((double)toPrec(p_amount.m_precision)) /
                         ((double)p_amount.m_baseDecimalNumber)) *
                        POW_10(p_amount.m_precision));
    m_precision = p_amount.m_precision;
  }

  return *this;
}

inline DecimalNumber DecimalNumber::operator+=(double p_amount) {
  m_baseDecimalNumber += (uint64_t)round(p_amount * POW_10(m_precision));
  return *this;
}

inline DecimalNumber DecimalNumber::operator-=(double p_amount) {
  m_baseDecimalNumber -= (uint64_t)round(p_amount * POW_10(m_precision));
  return *this;
}

inline DecimalNumber DecimalNumber::operator/=(double p_amount) {
  m_baseDecimalNumber =
      (uint64_t)round(((double)m_baseDecimalNumber) / p_amount);
  return *this;
}

inline DecimalNumber DecimalNumber::operator*=(double p_amount) {
  if (m_baseDecimalNumber == 0 || p_amount == 0.) {
    m_baseDecimalNumber = 0;
  } else {
    m_baseDecimalNumber =
        (uint64_t)round(((double)m_baseDecimalNumber) * p_amount);
  }

  return *this;
}

inline bool DecimalNumber::operator==(const DecimalNumber& p_amount) const {
  if (m_precision >= p_amount.m_precision) {
    return m_baseDecimalNumber == toSame(p_amount);
  } else {
    return p_amount.operator==(*this);
  }
}

inline bool DecimalNumber::operator>(const DecimalNumber& p_amount) const {
  if (m_precision >= p_amount.m_precision) {
    return m_baseDecimalNumber > toSame(p_amount);
  } else {
    return p_amount.operator<(*this);
  }
}

inline bool DecimalNumber::operator>=(const DecimalNumber& p_amount) const {
  if (m_precision >= p_amount.m_precision) {
    return m_baseDecimalNumber >= toSame(p_amount);
  } else {
    return p_amount.operator<=(*this);
  }
}

inline bool DecimalNumber::operator<(const DecimalNumber& p_amount) const {
  if (m_precision >= p_amount.m_precision) {
    return m_baseDecimalNumber < toSame(p_amount);
  } else {
    return p_amount.operator>(*this);
  }
}

inline bool DecimalNumber::operator<=(const DecimalNumber& p_amount) const {
  if (m_precision >= p_amount.m_precision) {
    return m_baseDecimalNumber <= toSame(p_amount);
  } else {
    return p_amount.operator>=(*this);
  }
}

inline bool DecimalNumber::operator!=(const DecimalNumber& p_amount) const {
  if (m_precision >= p_amount.m_precision) {
    return m_baseDecimalNumber != toSame(p_amount);
  } else {
    return p_amount.operator!=(*this);
  }
}

inline DecimalNumber DecimalNumber::operator=(const DecimalNumber& p_amount) {
  m_baseDecimalNumber = p_amount.m_baseDecimalNumber;
  m_precision = p_amount.m_precision;
  return *this;
}

inline std::string DecimalNumber::toStoreable() const {
  return absl::StrCat(m_baseDecimalNumber, "/", m_precision);
}

inline DecimalNumber DecimalNumber::fromStoreable(const std::string& _str) {
  std::vector<absl::string_view> splits = absl::StrSplit(_str, '/');
  DecimalNumber number;

  if (splits.empty()) {
    return number;
  }

  uint64_t base_number = 0;
  if (!absl::SimpleAtoi(splits[0], &base_number)) {
    return number;
  }

  if (splits.size() == 1) {
    number.m_baseDecimalNumber = base_number * POW_10(kDefaultPrecision);
    return number;
  } else if (int precision = 0; !absl::SimpleAtoi(splits[1], &precision)) {
    return number;
  } else {
    number.m_baseDecimalNumber = base_number;
    number.m_precision = precision;
    return number;
  }
}

inline DecimalNumber DecimalNumber::fromStoreable2(const std::string& _str,
                                                   int _numDigits) {
  std::vector<absl::string_view> splits = absl::StrSplit(_str, '/');
  double first = 0, second = 0;
  double am;

  if (!splits.empty() && !absl::SimpleAtod(splits[0], &first)) {
    first = 0;
  }
  if (splits.size() > 1 && !absl::SimpleAtod(splits[1], &second)) {
    second = 0;
  }

  if (splits.size() > 1) {
    am = first / second;
  } else {
    am = first;
  }

  return DecimalNumber(am, kDefaultPrecision);
}

// inline DecimalNumber DecimalNumber::fromUserLocale(const std::string& _str,
// int _numDigits)
// {
//     QLocale locale;
//     return DecimalNumber(locale.toDouble(_str), _numDigits);
// }

inline int DecimalNumber::precision() const { return m_precision; }

inline std::string DecimalNumber::toString() const {
  std::string num = absl::StrCat(
      m_baseDecimalNumber < 0 ? -m_baseDecimalNumber : m_baseDecimalNumber);

  while (static_cast<int>(num.length()) < m_precision + 1) {
    num.insert(0, "0");
  }

  int i = num.length() - m_precision;

  if (m_precision > 0) {
    num.insert(i, 1, kDecimalSeparator);
  }

  //        if (num.startsWith(kDecimalSeparator))
  //        {
  //            num.insert(0, "0");
  //        }

  // Group

  while (i - 3 > 0) {
    i -= 3;
    num.insert(i, 1, kGroupSeparator);
  }

  // Neg
  if (m_baseDecimalNumber < 0) {
    num.insert(0, "-");
  }

  return num;
}

inline double DecimalNumber::toDouble() const {
  return ((double)m_baseDecimalNumber) / POW_10(m_precision);
}

inline uint64_t DecimalNumber::roundToInt() const {
  return (uint64_t)round(toDouble());
}

inline DecimalNumber DecimalNumber::toPrecision(int _prec) const {
  DecimalNumber a(*this);
  a.m_baseDecimalNumber = toPrec(_prec);
  a.m_precision = _prec;
  return a;
}

inline const DecimalNumber operator+(const int p_amount1,
                                     const DecimalNumber& p_amount2) {
  return p_amount2 + (double)p_amount1;
}

inline const DecimalNumber operator+(const double p_amount1,
                                     const DecimalNumber& p_amount2) {
  return p_amount2 + p_amount1;
}

inline const DecimalNumber operator-(const int p_amount1,
                                     const DecimalNumber& p_amount2) {
  return (-p_amount2) + (double)p_amount1;
}

inline const DecimalNumber operator-(const double p_amount1,
                                     const DecimalNumber& p_amount2) {
  return (-p_amount2) + p_amount1;
}

inline const DecimalNumber operator*(const int p_amount1,
                                     const DecimalNumber& p_amount2) {
  return p_amount2 * (double)p_amount1;
}

inline const DecimalNumber operator*(const double p_amount1,
                                     const DecimalNumber& p_amount2) {
  return p_amount2 * p_amount1;
}

inline const DecimalNumber operator/(const int p_amount1,
                                     const DecimalNumber& p_amount2) {
  if (p_amount2 == 0) {
    return DecimalNumber(0, p_amount2.precision());
  } else {
    return p_amount2.inverse() * (double)p_amount1;
  }
}

inline const DecimalNumber operator/(const double p_amount1,
                                     const DecimalNumber& p_amount2) {
  if (p_amount2 == 0) {
    return DecimalNumber(0, p_amount2.precision());
  } else {
    return p_amount2.inverse() * p_amount1;
  }
}
}  // namespace kangaroo

#endif  // ENGINE_MODEL_DECIMAL_NUMBER_HPP
