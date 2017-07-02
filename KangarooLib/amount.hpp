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

#ifndef AMOUNT_HPP
#define AMOUNT_HPP

#include <QLocale>
#include <cmath>
#include <string>

//#define POW_10(a) pow(10.0,a)
#define POW_10(a) POW[a]

namespace KLib {

    inline Amount Amount::inverse() const
    {
        Amount a = *this;
        a.m_baseAmount = (qint64) round(POW_10(a.m_precision) / (double) a.m_baseAmount);
        return a;
    }

    inline void Amount::clear()
    {
        m_baseAmount = 0;
    }

    inline Amount Amount::abs() const
    {
        Amount a = *this;
        a.m_baseAmount = std::abs(a.m_baseAmount);
        return a;
    }

    inline Amount Amount::operator+(const Amount & p_amount) const
    {
        return Amount(*this) += p_amount;
    }

    inline Amount Amount::operator-(const Amount & p_amount) const
    {
        return Amount(*this) -= p_amount;
    }

    inline Amount Amount::operator-() const
    {
        Amount a = *this;
        a.m_baseAmount = -a.m_baseAmount;
        return a;
    }

    inline Amount Amount::operator*(const Amount & p_amount) const
    {
        return Amount(*this) *= p_amount;
    }

    inline Amount Amount::operator/(const Amount & p_amount) const
    {
        return Amount(*this) /= p_amount;
    }

    inline Amount Amount::operator+(double p_amount) const
    {
        return Amount(*this) += p_amount;
    }

    inline Amount Amount::operator-(double p_amount) const
    {
        return Amount(*this) -= p_amount;
    }

    inline Amount Amount::operator*(double p_amount) const
    {
        return Amount(*this) *= p_amount;
    }

    inline Amount Amount::operator/(double p_amount) const
    {
        return Amount(*this) /= p_amount;
    }

    inline qint64 Amount::toSame(const Amount& _other) const
    {
        return (_other.m_precision == m_precision)
                ? _other.m_baseAmount
                : (qint64) round(pow(10.0, m_precision - _other.m_precision) * (double) _other.m_baseAmount);
    }


    inline qint64 Amount::toPrec(int _prec) const
    {
        return (m_precision == (int)_prec) ? m_baseAmount :
                                        (qint64) round(pow(10.0, _prec - m_precision) * (double) m_baseAmount);
    }

    inline Amount Amount::operator+=(const Amount & p_amount)
    {
        if (m_precision >= p_amount.m_precision)
        {
            m_baseAmount += toSame(p_amount);
        }
        else
        {
            m_baseAmount = toPrec(p_amount.m_precision) + p_amount.m_baseAmount;
            m_precision = p_amount.m_precision;
        }

        return *this;
    }

    inline Amount Amount::operator-=(const Amount & p_amount)
    {
        if (m_precision >= p_amount.m_precision)
        {
            m_baseAmount -= toSame(p_amount);
        }
        else
        {
            m_baseAmount = toPrec(p_amount.m_precision) - p_amount.m_baseAmount;
            m_precision = p_amount.m_precision;
        }

        return *this;
    }

    inline Amount Amount::operator*=(const Amount & p_amount)
    {
        if (m_baseAmount == 0 || p_amount.m_baseAmount == 0)
        {
            m_baseAmount = 0;
        }
        else
        {
            if (m_precision >= p_amount.m_precision)
            {
                m_baseAmount *= toSame(p_amount);
                m_baseAmount /= POW_10(m_precision);
            }
            else
            {
                m_baseAmount = toPrec(p_amount.m_precision) * p_amount.m_baseAmount;
                m_precision = p_amount.m_precision;
                m_baseAmount /= POW_10(m_precision);
            }
        }

        return *this;
    }

    inline Amount Amount::operator/=(const Amount & p_amount)
    {
        if (m_precision >= p_amount.m_precision)
        {
            m_baseAmount = (qint64) round((((double) m_baseAmount)
                                           / ((double) toSame(p_amount)))
                                          * POW_10(m_precision));
        }
        else
        {
            m_baseAmount = (qint64) round((((double) toPrec(p_amount.m_precision))
                                         / ((double) p_amount.m_baseAmount))
                                        * POW_10(p_amount.m_precision));
            m_precision = p_amount.m_precision;
        }

        return *this;
    }

    inline Amount Amount::operator+=(double p_amount)
    {
        m_baseAmount += (qint64) round(p_amount * POW_10(m_precision));
        return *this;
    }

    inline Amount Amount::operator-=(double p_amount)
    {
        m_baseAmount -= (qint64) round(p_amount * POW_10(m_precision));
        return *this;
    }

    inline Amount Amount::operator/=(double p_amount)
    {
        m_baseAmount = (qint64) round(((double) m_baseAmount) / p_amount);
        return *this;
    }

    inline Amount Amount::operator*=(double p_amount)
    {
        if (m_baseAmount == 0 || p_amount == 0.)
        {
            m_baseAmount = 0;
        }
        else
        {
            m_baseAmount = (qint64) round(((double) m_baseAmount) * p_amount);
        }

        return *this;
    }

    inline bool Amount::operator==(const Amount & p_amount) const
    {
        if (m_precision >= p_amount.m_precision)
        {
            return m_baseAmount == toSame(p_amount);
        }
        else
        {
            return p_amount.operator==(*this);
        }
    }

    inline bool Amount::operator>(const Amount & p_amount) const
    {
        if (m_precision >= p_amount.m_precision)
        {
            return m_baseAmount > toSame(p_amount);
        }
        else
        {
            return p_amount.operator<(*this);
        }
    }

    inline bool Amount::operator>=(const Amount & p_amount) const
    {
        if (m_precision >= p_amount.m_precision)
        {
            return m_baseAmount >= toSame(p_amount);
        }
        else
        {
            return p_amount.operator<=(*this);
        }
    }

    inline bool Amount::operator<(const Amount & p_amount) const
    {
        if (m_precision >= p_amount.m_precision)
        {
            return m_baseAmount < toSame(p_amount);
        }
        else
        {
            return p_amount.operator>(*this);
        }
    }

    inline bool Amount::operator<=(const Amount & p_amount) const
    {
        if (m_precision >= p_amount.m_precision)
        {
            return m_baseAmount <= toSame(p_amount);
        }
        else
        {
            return p_amount.operator>=(*this);
        }
    }

    inline bool Amount::operator!=(const Amount & p_amount) const
    {
        if (m_precision >= p_amount.m_precision)
        {
            return m_baseAmount != toSame(p_amount);
        }
        else
        {
            return p_amount.operator!=(*this);
        }
    }

    inline Amount Amount::operator=(const Amount & p_amount)
    {
        m_baseAmount = p_amount.m_baseAmount;
        m_precision = p_amount.m_precision;
        return *this;
    }

    inline QString Amount::toStoreable() const
    {
        return QString::number(m_baseAmount) + "/" + QString::number(m_precision);
    }

    inline QVariant Amount::toQVariant() const
    {
        return QVariant(toStoreable());
    }

    inline Amount Amount::fromQVariant(const QVariant& _v)
    {
        return fromStoreable(_v.toString());
    }

    inline Amount Amount::fromStoreable(const QString& _str)
    {
        QString first,second;
        Amount a;

        if (_str.isEmpty())
        {
            return a;
        }

        first = _str.section('/', 0,0);
        second = _str.section('/',1,1);

        if (!second.isEmpty())
        {
            a.m_baseAmount = std::stoll(first.toStdString());
            a.m_precision = second.toInt();
        }
        else
        {
            a.m_baseAmount = first.toInt() * POW_10(a.m_precision);
        }

        return a;
    }

    inline Amount Amount::fromStoreable2(const QString& _str, int _numDigits)
    {
        QString first,second;
        double am;

        first = _str.section('/', 0,0);
        second = _str.section('/',1,1);

        if (!second.isEmpty())
        {
            am = first.toDouble() / second.toDouble();
        }
        else
        {
            am = first.toDouble();
        }

        return Amount(am, _numDigits);
    }

    inline Amount Amount::fromUserLocale(const QString& _str, int _numDigits)
    {
        QLocale locale;
        return Amount(locale.toDouble(_str), _numDigits);
    }

    inline int Amount::precision() const
    {
        return m_precision;
    }

    inline QString Amount::toString() const
    {
        QString num = QString::number(m_baseAmount < 0 ? -m_baseAmount : m_baseAmount);

        while (num.length() < m_precision+1)
        {
            num.insert(0, "0");
        }

        int i = num.length()-m_precision;

        if (m_precision > 0)
        {
            num.insert(i, DECIMAL_SEPARATOR);
        }

//        if (num.startsWith(DECIMAL_SEPARATOR))
//        {
//            num.insert(0, "0");
//        }

        //Group

        while (i-3 > 0)
        {
            i-= 3;
            num.insert(i, GROUP_SEPARATOR);
        }

        //Neg
        if (m_baseAmount < 0)
        {
            num.insert(0, "-");
        }

        return num;

//        QLocale locale;
//        return locale.toString(toDouble(), 'f', m_precision);
    }

    inline double Amount::toDouble() const
    {
        return ((double) m_baseAmount) / POW_10(m_precision);
    }

    inline qint64 Amount::roundToInt() const
    {
        return (qint64) round(toDouble());
    }

    inline Amount Amount::toPrecision(int _prec) const
    {
        Amount a(*this);
        a.m_baseAmount = toPrec(_prec);
        a.m_precision = _prec;
        return a;
    }

    inline const Amount operator+(const int p_amount1, const Amount & p_amount2)
    {
        return p_amount2 + (double) p_amount1;
    }

    inline const Amount operator+(const double p_amount1, const Amount & p_amount2)
    {
        return p_amount2 + p_amount1;
    }

    inline const Amount operator-(const int p_amount1, const Amount & p_amount2)
    {
        return (-p_amount2) + (double)p_amount1;
    }

    inline const Amount operator-(const double p_amount1, const Amount & p_amount2)
    {
        return (-p_amount2) + p_amount1;
    }

    inline const Amount operator*(const int p_amount1, const Amount & p_amount2)
    {
        return p_amount2 * (double)p_amount1;
    }

    inline const Amount operator*(const double p_amount1, const Amount & p_amount2)
    {
        return p_amount2 * p_amount1;
    }

    inline const Amount operator/(const int p_amount1, const Amount & p_amount2)
    {
        if (p_amount2 == 0)
        {
            return Amount(0, p_amount2.precision());
        }
        else
        {
            return p_amount2.inverse() * (double)p_amount1;
        }
    }

    inline const Amount operator/(const double p_amount1, const Amount & p_amount2)
    {
        if (p_amount2 == 0)
        {
            return Amount(0, p_amount2.precision());
        }
        else
        {
            return p_amount2.inverse() * p_amount1;
        }
    }
}

#endif // AMOUNT_HPP
