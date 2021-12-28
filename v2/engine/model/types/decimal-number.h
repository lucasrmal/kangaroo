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

/**
  @file  amount.h
  This file contains the method declarations for the DecimalNumber class

  @author   Lucas Rioux Maldague
  @date     August 30th 2009
  @version  3.0

*/

#ifndef ENGINE_MODEL_DECIMAL_NUMBER_H
#define ENGINE_MODEL_DECIMAL_NUMBER_H

#include <string>

namespace kangaroo {

class DecimalNumber {
 public:
  DecimalNumber() : m_baseDecimalNumber(0), m_precision(2) {}

  DecimalNumber(const int _int, int _precision = 2);
  DecimalNumber(const double _double, int _precision = 2);
  explicit DecimalNumber(const std::string& _string, int _precision = 2);

  DecimalNumber(const DecimalNumber& _amount)
      : m_baseDecimalNumber(_amount.m_baseDecimalNumber),
        m_precision(_amount.m_precision) {}

  DecimalNumber inverse() const;
  DecimalNumber abs() const;

  DecimalNumber operator+(const DecimalNumber& p_amount) const;
  DecimalNumber operator-(const DecimalNumber& p_amount) const;
  DecimalNumber operator*(const DecimalNumber& p_amount) const;
  DecimalNumber operator/(const DecimalNumber& p_amount) const;
  DecimalNumber operator+(double p_amount) const;
  DecimalNumber operator-(double p_amount) const;
  DecimalNumber operator*(double p_amount) const;
  DecimalNumber operator/(double p_amount) const;
  DecimalNumber operator-() const;

  DecimalNumber operator+=(const DecimalNumber& p_amount);
  DecimalNumber operator-=(const DecimalNumber& p_amount);
  DecimalNumber operator*=(const DecimalNumber& p_amount);
  DecimalNumber operator/=(const DecimalNumber& p_amount);
  DecimalNumber operator+=(double p_amount);
  DecimalNumber operator-=(double p_amount);
  DecimalNumber operator*=(double p_amount);
  DecimalNumber operator/=(double p_amount);

  bool operator==(const DecimalNumber& p_amount) const;
  bool operator>(const DecimalNumber& p_amount) const;
  bool operator>=(const DecimalNumber& p_amount) const;
  bool operator<(const DecimalNumber& p_amount) const;
  bool operator<=(const DecimalNumber& p_amount) const;
  bool operator!=(const DecimalNumber& p_amount) const;

  DecimalNumber operator=(const DecimalNumber& p_amount);

  void clear();

  std::string toString() const;
  double toDouble() const;
  uint64_t roundToInt() const;
  int precision() const;
  DecimalNumber toPrecision(int _prec) const;

  std::string toStoreable() const;
  static DecimalNumber fromStoreable(const std::string& _str);
  static DecimalNumber fromStoreable2(const std::string& _str,
                                      int _numDigits = 2);
  // static DecimalNumber fromUserLocale(const std::string& _str,
  //                                     int _numDigits = 2);

  static constexpr int kDefaultPrecision = 2;
  static constexpr int kMaxDecimals = 6;

 private:
  uint64_t m_baseDecimalNumber;
  int m_precision = kDefaultPrecision;

  uint64_t toSame(const DecimalNumber& _other) const;
  uint64_t toPrec(int _prec) const;

  static const double POW[kMaxDecimals + 1];

  static constexpr char kDecimalSeparator = '.';
  static constexpr char kGroupSeparator = ',';
};

const DecimalNumber operator+(const int p_amount1,
                              const DecimalNumber& p_amount2);
const DecimalNumber operator+(const double p_amount1,
                              const DecimalNumber& p_amount2);

const DecimalNumber operator-(const int p_amount1,
                              const DecimalNumber& p_amount2);
const DecimalNumber operator-(const double p_amount1,
                              const DecimalNumber& p_amount2);

const DecimalNumber operator*(const int p_amount1,
                              const DecimalNumber& p_amount2);
const DecimalNumber operator*(const double p_amount1,
                              const DecimalNumber& p_amount2);

const DecimalNumber operator/(const int p_amount1,
                              const DecimalNumber& p_amount2);
const DecimalNumber operator/(const double p_amount1,
                              const DecimalNumber& p_amount2);

}  // namespace kangaroo

#include "model/types/decimal-number.hpp"

#endif  // ENGINE_MODEL_DECIMAL_NUMBER_H
