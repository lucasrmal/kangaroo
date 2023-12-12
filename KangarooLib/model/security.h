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

#ifndef SECURITY_H
#define SECURITY_H

namespace KLib
{
    enum class SecurityInfo;
}


unsigned int qHash(KLib::SecurityInfo _key, unsigned int seed = 0);

#include <QHash>
#include <QVector>
#include "stored.h"
#include "../interfaces/scriptable.h"
#include "../amount.h"

namespace KLib
{
    enum class SecurityType
    {
        Stock = 1,
        PreferredStock = 2,

        ETF = 10,
        MutualFund = 11,

        Bond = 20,

        Index = 50
    };

    class Account;

    enum class SecurityInfo
    {
        AskPrice             = 100,
        BidPrice             = 101,
        OpenPrice            = 102,
        PreviousClosePrice   = 103,

        AverageDailyVolume   = 200,
        Volume               = 201,

        BookValuePerShare    = 300,
        DividendPerShare     = 301,
        DividendYield        = 302,
        EPS                  = 303,
        EPSEstimateCurYear   = 304,
        EPSEstimateNextYear  = 305,
        EBITDA               = 306,

        PriceToBook          = 400,
        PriceToSales         = 401,
        PEGRatio             = 402,
        ShortRatio           = 403,
        PERatio              = 404,

        DividendPayDate      = 500,
        ExDividendDate       = 501,

        MarketCap            = 600,

        DayChange            = 700,
        DayHigh              = 701,
        DayLow               = 702,
        YearHigh             = 703,
        YearLow              = 704,
        FiftyDayAverage      = 705,
        TwoHundredDayAverage = 706
    };


    /**
     * @brief The Security class
     *
     * market and sector are stock-only properties.
     */
    class Security : public IStored
    {
        Q_OBJECT
        K_SCRIPTABLE(Security)

        Q_PROPERTY(QString name READ name WRITE setName)
        Q_PROPERTY(QString symbol READ symbol WRITE setSymbol)
        Q_PROPERTY(QString market READ market WRITE setMarket)
        Q_PROPERTY(QString sector READ sector WRITE setSector)
        Q_PROPERTY(QString currency READ currency WRITE setCurrency)
        Q_PROPERTY(unsigned int precision READ precision WRITE setPrecision)
        Q_PROPERTY(int idPicture READ idPicture WRITE setIdPicture)
        Q_PROPERTY(bool defaultToSecurityPicture READ defaultToSecurityPicture WRITE setDefaultToSecurityPicture)
        Q_PROPERTY(QString note READ note WRITE setNote)

        Security();

        public:

            virtual ~Security() {}

            SecurityType type() const        { return m_type; }
            QString      name() const        { return m_name; }
            QString      symbol() const      { return m_symbol; }
            QString      market() const      { return m_market; }
            QString      sector() const      { return m_sector; }
            QString      geoLocation() const { return m_geoLocation; }
            QString      currency() const    { return m_currency; }
            unsigned int precision() const   { return m_precision; }
            int          idPicture() const   { return m_idPicture; }
            int          idProvider() const  { return m_idProvider; }
            QString      note() const        { return m_note; }
            bool         defaultToSecurityPicture() const { return m_defaultToPicture; }
            QString      formattedName() const;

            Amount       securityInfo(SecurityInfo _info);

            void         setType(SecurityType _type);
            void         setName(const QString& _name);
            void         setSymbol(const QString& _symbol);
            void         setMarket(const QString& _market);
            void         setSector(const QString& _sector);
            void         setGeoLocation(const QString& _location);
            void         setCurrency(const QString& _currency);
            void         setPrecision(unsigned int _precision);
            void         setIdPicture(int _id);
            void         setIdProvider(int _id);
            void         setDefaultToSecurityPicture(bool _value);
            void         setNote(const QString& _note);

            void         setSecurityInfo(SecurityInfo _info, const Amount& _amount);

            Q_INVOKABLE QString formatAmount(const Amount& _amount) const;


            Q_INVOKABLE bool canRemove() const;
            Q_INVOKABLE Account* tradingAccount() const;

            static QString typeToString(SecurityType _type);

        protected:
            void load(QXmlStreamReader& _reader) override;
            void save(QXmlStreamWriter& _writer) const override;

        private:
            SecurityType m_type;
            QString      m_name;
            QString      m_symbol;
            QString      m_market;
            QString      m_sector;
            QString      m_geoLocation;
            QString      m_currency;
            unsigned int m_precision;
            int          m_idPicture;
            int          m_idProvider;
            bool         m_defaultToPicture;
            QString      m_note;

            QHash<SecurityInfo, Amount> m_infos;

            Account* m_trading;

            friend class SecurityManager;
            friend class Account;
    };

    class SecurityManager : public IStored
    {
        Q_OBJECT
        K_SCRIPTABLE(SecurityManager)

        Q_PROPERTY(int securityCount READ securityCount)
        Q_PROPERTY(int indexCount READ indexCount)

        public:
            Q_INVOKABLE KLib::Security* add(SecurityType _type,
                                            const QString& _name,
                                            const QString& _symbol,
                                            const QString& _market,
                                            const QString& _sector,
                                            const QString& _currency,
                                            unsigned int _precision = 2);

            Q_INVOKABLE KLib::Security* get(int _id) const;
            Q_INVOKABLE KLib::Security* get(const QString& _symbol) const;
            Q_INVOKABLE KLib::Security* securityAt(int _i) const;
            Q_INVOKABLE KLib::Security* indexAt(int _i) const;
            Q_INVOKABLE void remove(int _id);

            Q_INVOKABLE QStringList sectors() const;
            Q_INVOKABLE QStringList markets() const;

            QHash<QString, QList<KLib::Security*> > stocksBySector() const;
            QHash<QString, QList<KLib::Security*> > stocksByMarket() const;


            int securityCount() const { return m_securities.size(); }
            int indexCount() const    { return m_indexes.size(); }

            Q_INVOKABLE const QVector<KLib::Security*>& securities() const  { return m_securities; }
            Q_INVOKABLE const QVector<KLib::Security*>& indexes() const     { return m_indexes; }

            static SecurityManager* instance() { return m_instance; }

        signals:
            void securityAdded(Security* s);
            void securityRemoved(Security* s);
            void securityModified(Security* s);

            void indexAdded(Security* s);
            void indexRemoved(Security* s);
            void indexModified(Security* s);

        protected slots:
            void onModified();

        protected:
            void load(QXmlStreamReader& _reader) override;
            void save(QXmlStreamWriter& _writer) const override;
            void unload() override;

        private:
            enum class SecurityClass
            {
                Security,
                Index
            };

            typedef QPair<SecurityClass, int> IndexLocation;

            QHash<int, IndexLocation> m_index;
            QVector<Security*> m_securities;
            QVector<Security*> m_indexes;

            static SecurityManager* m_instance;
            static int m_nextId;
    };

}

Q_DECLARE_METATYPE(KLib::Security*)
Q_DECLARE_METATYPE(KLib::SecurityManager*)

#endif // SECURITY_H
