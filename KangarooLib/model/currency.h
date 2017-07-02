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

#ifndef CURRENCY_H
#define CURRENCY_H

#include <QVector>
#include <QHash>
#include "stored.h"
#include "../interfaces/scriptable.h"

namespace KLib
{
    class Account;
    class Amount;

    class Currency : public IStored
    {
        Q_OBJECT
        K_SCRIPTABLE(Currency)

        Q_PROPERTY(QString name READ name WRITE setName)
        Q_PROPERTY(QString code READ code)
        Q_PROPERTY(QString customSymbol READ customSymbol WRITE setCustomSymbol)
        Q_PROPERTY(unsigned int precision READ precision WRITE setPrecision)

        public:

            Currency();

            QString name() const   { return m_name; }
            QString code() const   { return m_code; }
            QString customSymbol() const { return m_symbol; }
            QString symbol() const { return m_symbol.isEmpty() ? m_code : m_symbol; }
            unsigned int precision() const { return m_precision; }

            void setName(const QString& _name);
            void setCustomSymbol(const QString& _symbol);
            void setPrecision(unsigned int _precision);

            Q_INVOKABLE bool canRemove() const;

            Q_INVOKABLE Account* tradingAccount() const;

            Q_INVOKABLE QString formatAmount(const Amount& _amount) const;

        protected:
            void setCode(const QString& _code);

            void load(QXmlStreamReader& _reader) override;
            void save(QXmlStreamWriter& _writer) const override;

        private:
            QString m_code;
            QString m_name;
            QString m_symbol;
            unsigned int m_precision;

            Account* m_trading;

            friend class CurrencyManager;
            friend class Account;
    };

    class CurrencyManager : public IStored
    {
        Q_OBJECT
        K_SCRIPTABLE(CurrencyManager)

        Q_PROPERTY(int count READ count)

        public:

            struct WorldCurrency
            {
                WorldCurrency() : precision(2) {}
                WorldCurrency(const QString _name,
                              const QString _code,
                              const QString _symbol,
                              unsigned int _precision = 2) :
                       name(_name),
                       code(_code),
                       symbol(_symbol),
                       precision(_precision) {}

                QString name;
                QString code;
                QString symbol;
                unsigned int precision;
            };

            Q_INVOKABLE KLib::Currency* add(const QString& _code,
                                            const QString& _name,
                                            const QString& _customSymbol = QString(),
                                            unsigned int _precision = 2);
            Q_INVOKABLE KLib::Currency* get(const QString& _code) const;
            Q_INVOKABLE KLib::Currency* get(const QString& _code, bool _add);

            Q_INVOKABLE bool has(const QString& _code) const;

            Q_INVOKABLE KLib::Currency* at(int _i) const;
            Q_INVOKABLE void remove(const QString &_code);

            Q_INVOKABLE const QVector<KLib::Currency*>& currencies() const { return m_currencies; }

            int count() const { return m_currencies.size(); }

            static CurrencyManager* instance();

            static QList<WorldCurrency> worldCurrencies();

        signals:
            void currencyAdded(KLib::Currency* c);
            void currencyRemoved(KLib::Currency* c);
            void currencyModified(KLib::Currency* c);

        protected slots:
            void onModified();

        protected:
            void load(QXmlStreamReader& _reader) override;
            void save(QXmlStreamWriter& _writer) const override;
            void loadNew() override;
            void unload() override;

        private:
            void addDefaultCurrency();

            QHash<QString, int> m_index;
            QVector<Currency*> m_currencies;

            static CurrencyManager* m_instance;
    };

}

Q_DECLARE_METATYPE(KLib::Currency*)
Q_DECLARE_METATYPE(KLib::CurrencyManager*)

#endif // CURRENCY_H
