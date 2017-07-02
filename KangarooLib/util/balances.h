#ifndef BALANCES_H
#define BALANCES_H

#include "../amount.h"
#include <QHash>
#include <QDate>

namespace KLib
{
    class Balances
    {
        public:
            Balances() {}

            Balances(const QString& _cur, const Amount& _a);

            Balances& operator+=(const Balances& _other);
            Balances& operator-=(const Balances& _other);

            bool operator<(const Amount& _other) const
            {
                for (auto a : bal)
                {
                    if (a >= _other)
                        return false;
                }

                return true;
            }

            Balances operator+(const Balances& _other) const
            {
                Balances b = *this;
                b += _other;
                return b;
            }

            Balances operator-(const Balances& _other) const
            {
                Balances b = *this;
                b -= _other;
                return b;
            }

            Amount  inCurrency(const QString& _currency, const QDate& _date = QDate()) const;

            void    add(const QString& _cur, const Amount& _a);

            const Amount operator[](const QString& _cur) const           { return bal[_cur]; }
            Amount  value(const QString& _cur) const                { return bal.value(_cur); }

            bool    operator==(const Balances& _other) const;
            int     count() const                                   { return bal.count(); }
            bool    isEmpty() const                                 { return bal.isEmpty(); }
            bool    contains(const QString& _cur) const             { return bal.contains(_cur); }

            QHash<QString, Amount>::const_iterator begin() const    { return bal.begin(); }
            QHash<QString, Amount>::const_iterator end() const      { return bal.end(); }

        private:
            QHash<QString, Amount> bal;
    };
}

#endif // BALANCES_H
