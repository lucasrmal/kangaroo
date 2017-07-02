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

#ifndef LEDGERCONTROLLERMANAGER_H
#define LEDGERCONTROLLERMANAGER_H

#include <QList>
#include <QHash>
#include <QString>
#include <functional>

class QObject;

namespace KLib
{
    class LedgerController;
    class Ledger;
    class Account;

    class LedgerControllerFactorySuper
    {
        public:
            virtual LedgerController* buildLedgerController(Ledger* _ledger, QObject* _parent = 0) const = 0;

            QString code;
            std::function<bool(const Account*)> handles;
            int defaultPriority;
    };

    template <class T>
    class LedgerControllerFactory : public LedgerControllerFactorySuper
    {

        public:
            LedgerControllerFactory(const QString& _code,
                                    std::function<bool(const Account*)> _handles,
                                    int _priority = 0)
            {
                handles = _handles;
                code = _code;
                defaultPriority = _priority;
            }

            LedgerController* buildLedgerController(Ledger* _ledger, QObject* _parent = 0) const
            {
                return new T(_ledger, _parent);
            }

    };

    class LedgerControllerManager
    {
            LedgerControllerManager();
        public:
            static LedgerControllerManager* instance() { return m_instance; }

            /**
             * @brief Registers a new ledgercontroller
             * @param _code The unique identifier for the ledgercontroller
             * @param _types The account types handled by this ledgercontroller. Empty => Can do all of them.
             * @param _controller
             */
            void registerLedger(LedgerControllerFactorySuper* _controller);

            /**
             * @brief ledgerFor
             * @param _type
             * @return The ledgercontroller most specific for type, or NULL if no ledger can handle this type
             */
            LedgerController* ledgerFor(const Account* _account, QObject* _parent = 0) const;

        private:
            QHash<QString, LedgerControllerFactorySuper*> m_controllers;
            QList<LedgerControllerFactorySuper*> m_sorted;

            static LedgerControllerManager* m_instance;
    };

}

#endif // LEDGERCONTROLLERMANAGER_H
