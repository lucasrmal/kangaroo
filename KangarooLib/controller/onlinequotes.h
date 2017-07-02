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

#ifndef ONLINEQUOTES_H
#define ONLINEQUOTES_H

#include <QString>
#include <QHash>
#include <QObject>
#include "../interfaces/iquote.h"

namespace KLib
{
    enum class SecurityInfo;

    class OnlineQuotes : public QObject
    {
        Q_OBJECT

        public:
            ~OnlineQuotes();

            void    registerQuoteSource(IQuote* _source, bool _default = false);
            void    setDefault(const QString& _id);

            IQuote* get(const QString& _id) const;
            IQuote* getDefault() const;

            QList<IQuote*> quoteSources() const { return m_sources.values(); }

            bool    hasDefault() const { return !m_default.isEmpty(); }

            void    updateAll();

            static OnlineQuotes* instance() { return m_instance; }

        signals:
            void quoteReady(const KLib::PricePair& _pair, double _quote);
            void requestError(const QString& _message);
            void requestDone();

        private slots:
            void onQuoteReady(IQuote::Type _type,
                              const PricePair& _pair,
                              double _quote,
                              const QHash<SecurityInfo, Amount> _extraInfos = QHash<SecurityInfo, Amount>());

            void onRequestError(int _id, const QString& _message);
            void onRequestDone(int _id);

        private:
            OnlineQuotes() {}

            QHash<QString, IQuote*> m_sources;
            QString m_default;

            QList<int> m_currentUpdate;

            static OnlineQuotes* m_instance;
    };

}

#endif // ONLINEQUOTES_H
