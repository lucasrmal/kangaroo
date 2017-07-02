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

#include "amount.h"
#include <stdexcept>

namespace KLib
{
    const double Amount::POW[Amount::MAX_PRECISION+1] = { 1., 10., 100., 1000., 10000., 100000., 1000000. };

    const QChar Amount::DECIMAL_SEPARATOR = QLocale().decimalPoint();
    const QChar Amount::GROUP_SEPARATOR = QLocale().groupSeparator();

    Amount::Amount(const int _int, int _precision) :
        m_baseAmount((qint64) (POW[_precision] * _int)),
        m_precision(_precision)
    {
        if (_precision > MAX_PRECISION)
        {
            throw std::invalid_argument("Precision cannot be larger than " + std::to_string(MAX_PRECISION));
        }
        else if (_precision < 0)
        {
            throw std::invalid_argument("Precision has to be at least 0.");
        }
    }

    Amount::Amount(const double _double, int _precision) :
        m_baseAmount((qint64) round(_double * POW[_precision])),
        m_precision(_precision)
    {
        if (_precision > MAX_PRECISION)
        {
            throw std::invalid_argument("Precision cannot be larger than " + std::to_string(MAX_PRECISION));
        }
        else if (_precision < 0)
        {
            throw std::invalid_argument("Precision has to be at least 0.");
        }
    }

    Amount::Amount(const QString & p_string, int _precision) :
        m_baseAmount((qint64) round(p_string.toDouble() * POW[_precision])),
        m_precision(_precision)
    {
        if (_precision > MAX_PRECISION)
        {
            throw std::invalid_argument("Precision cannot be larger than " + std::to_string(MAX_PRECISION));
        }
        else if (_precision < 0)
        {
            throw std::invalid_argument("Precision has to be at least 0.");
        }
    }
}
