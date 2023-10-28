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
  This file contains the method declarations for the Amount class

  @author   Lucas Rioux Maldague
  @date     August 30th 2009
  @version  3.0

*/

#ifndef AMOUNT_H
#define AMOUNT_H

#include <QVariant>

namespace KLib {

    /**
      The 'Amount' class represent a 'currency-style' number (2 precision decimals)
    */
    class Amount
    {
        public:
                        Amount() :
                          m_baseAmount(0),
                          m_precision(2) {}

                        Amount(const int _int, int _precision=2);
                        Amount(const double _double, int _precision=2);
               explicit Amount(const QString& _string, int _precision=2);

                        Amount(const Amount& _amount) :
                            m_baseAmount(_amount.m_baseAmount),
                            m_precision(_amount.m_precision) {}

            Q_INVOKABLE Amount      inverse() const;
            Q_INVOKABLE Amount      abs() const;

            Amount      operator+(const Amount & p_amount) const;
            Amount      operator-(const Amount & p_amount) const;
            Amount      operator*(const Amount & p_amount) const;
            Amount      operator/(const Amount & p_amount) const;
            Amount      operator+(double p_amount) const;
            Amount      operator-(double p_amount) const;
            Amount      operator*(double p_amount) const;
            Amount      operator/(double p_amount) const;
            Amount      operator-() const;

            Amount      operator+=(const Amount & p_amount);
            Amount      operator-=(const Amount & p_amount);
            Amount      operator*=(const Amount & p_amount);
            Amount      operator/=(const Amount & p_amount);
            Amount      operator+=(double p_amount);
            Amount      operator-=(double p_amount);
            Amount      operator*=(double p_amount);
            Amount      operator/=(double p_amount);

            bool        operator==(const Amount & p_amount) const;
            bool        operator>(const Amount & p_amount) const;
            bool        operator>=(const Amount & p_amount) const;
            bool        operator<(const Amount & p_amount) const;
            bool        operator<=(const Amount & p_amount) const;
            bool        operator!=(const Amount & p_amount) const;

            Amount      operator=(const Amount & p_amount);

            void        clear();

            Q_INVOKABLE QString toString() const;
            Q_INVOKABLE double  toDouble() const;
            Q_INVOKABLE qint64  roundToInt() const;
            Q_INVOKABLE int     precision() const;
            Q_INVOKABLE Amount  toPrecision(int _prec) const;

            QVariant    toQVariant() const;
            QString     toStoreable() const;
            static Amount fromQVariant(const QVariant& _v);
            static Amount fromStoreable(const QString& _str);
            static Amount fromStoreable2(const QString& _str, int _numDigits=2);
            static Amount fromUserLocale(const QString& _str, int _numDigits=2);

            static const int MAX_PRECISION = 6;

        private:
            qint64      m_baseAmount;
            int         m_precision;

            qint64      toSame(const Amount& _other) const;
            qint64      toPrec(int _prec) const;

            static const double POW[MAX_PRECISION+1];

            static const QChar DECIMAL_SEPARATOR;
            static const QChar GROUP_SEPARATOR;

    };

    const Amount operator+(const int p_amount1, const Amount & p_amount2);
    const Amount operator+(const double p_amount1, const Amount & p_amount2);

    const Amount operator-(const int p_amount1, const Amount & p_amount2);
    const Amount operator-(const double p_amount1, const Amount & p_amount2);

    const Amount operator*(const int p_amount1, const Amount & p_amount2);
    const Amount operator*(const double p_amount1, const Amount & p_amount2);

    const Amount operator/(const int p_amount1, const Amount & p_amount2);
    const Amount operator/(const double p_amount1, const Amount & p_amount2);    

}

Q_DECLARE_METATYPE(KLib::Amount)

#include "amount.hpp"

#endif // AMOUNT_H
