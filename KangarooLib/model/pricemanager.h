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

#ifndef PRICEMANAGER_H
#define PRICEMANAGER_H

#include "stored.h"
#include "../amount.h"
//#include "../util/treapmap.h"
#include "../interfaces/scriptable.h"

#include <QHash>
#include <QPair>
#include <QDate>
#include <QVector>

namespace KLib
{
    typedef QPair<QString,QString> PricePair;

    class Currency;
    class Security;

    class ExchangePair : public IStored
    {
        Q_OBJECT
        K_SCRIPTABLE(ExchangePair)

        Q_PROPERTY(int count READ count)
        Q_PROPERTY(bool isSecurity READ isSecurity)
        Q_PROPERTY(QString to READ to)
        Q_PROPERTY(QString from READ from)
        Q_PROPERTY(QString updateSource READ updateSource WRITE setUpdateSource)
        Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate)

        public:

            ExchangePair() : m_autoUpdate(true), m_security(nullptr) {}

            typedef QPair<QDate, double> Rate;

            Q_INVOKABLE void set(const QDate& _date, double _rate);
            Q_INVOKABLE void remove(const QDate& _date);

            Q_INVOKABLE double on(const QDate& _date) const;
            Q_INVOKABLE double last() const;

            int count() const { return m_rates.size(); }

            bool isSecurity() const { return m_from.size() > 3; }
            QString to() const { return m_to; }
            QString from() const { return m_from; }
            QString updateSource() const { return m_updateSource; }
            bool autoUpdate() const { return m_autoUpdate; }

            void setUpdateSource(const QString& _source);
            void setAutoUpdate(bool _auto);

            Q_INVOKABLE Security* securityFrom() const;

            Q_INVOKABLE const QList<Rate> rates() const;

        signals:
            void rateSet(const QDate& _date);
            void rateRemoved(const QDate& _date);

        protected:
            virtual void load(QXmlStreamReader& _reader) override;
            virtual void save(QXmlStreamWriter& _writer) const override;

        private:
            QString m_from;
            QString m_to;
            QString m_updateSource;
            bool    m_autoUpdate;

            mutable Security* m_security;

            std::map<QDate, double> m_rates;

            friend class PriceManager;
    };

    class PriceManager : public IStored
    {
        Q_OBJECT
        K_SCRIPTABLE(PriceManager)

        Q_PROPERTY(int count READ count)

        public:
            Q_INVOKABLE KLib::ExchangePair* add(const QString& _from, const QString& _to);
            Q_INVOKABLE KLib::ExchangePair* get(const QString& _from, const QString& _to) const;
            Q_INVOKABLE KLib::ExchangePair* getOrAdd(const QString& _from, const QString& _to);

            Q_INVOKABLE KLib::ExchangePair* at(int _i) const;
            Q_INVOKABLE void remove(const QString& _from, const QString& _to);
            Q_INVOKABLE void removeAll(const QString& _fromTo);

            int count() const { return m_pairs.size(); }

            Q_INVOKABLE const QVector<KLib::ExchangePair*>& pairs() const { return m_pairs; }

            Q_INVOKABLE double rate(int _idSecurity, const QString& _to, const QDate& _date = QDate()) const;
            Q_INVOKABLE double rate(const QString& _from, const QString& _to, const QDate& _date = QDate()) const;

            static PriceManager* instance() { return m_instance; }

            static QString securityId(int _idSecurity);

            static const unsigned int DECIMALS_RATE;

        signals:
            void exchangePairAdded(KLib::ExchangePair* _p);
            void exchangePairRemoved(KLib::ExchangePair* _p);

            void rateSet(KLib::ExchangePair* _p, const QDate& _date);
            void rateRemoved(KLib::ExchangePair* _p, const QDate& _date);
            void lastRateModified(KLib::ExchangePair* _p);

        private slots:
            void onRateSet(const QDate& _date);
            void onRateRemoved(const QDate& _date);


        protected:
            virtual void load(QXmlStreamReader& _reader) override;
            virtual void save(QXmlStreamWriter& _writer) const override;
            virtual void unload() override;

        private:
            PriceManager();

            QVector<ExchangePair*> m_pairs;
            QHash<PricePair, int> m_index;

            bool m_noEmit;

            static PriceManager* m_instance;
    };

}

Q_DECLARE_METATYPE(KLib::ExchangePair*)
Q_DECLARE_METATYPE(KLib::PriceManager*)

#endif // PRICEMANAGER_H
