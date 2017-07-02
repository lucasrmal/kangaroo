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

#include "ledgercontrollermanager.h"
#include "genericledgercontroller.h"
#include "investmentledgercontroller.h"
#include "brokerageledgercontroller.h"
#include "../../model/account.h"

#include <QDebug>

namespace KLib
{
    LedgerControllerManager* LedgerControllerManager::m_instance = new LedgerControllerManager();

    LedgerControllerManager::LedgerControllerManager()
    {
        //Register the default ledgers
        registerLedger(new LedgerControllerFactory<InvestmentLedgerController>("investment",
                       [](const Account* _a) { return _a->type() == AccountType::INVESTMENT; },
                       50));

//        registerLedger(new LedgerControllerFactory<BrokerageLedgerController>("brokerage",
//                       [](const Account* _a) { return _a->type() == AccountType::BROKERAGE; },
//                       49));

        registerLedger(new LedgerControllerFactory<GenericLedgerController>("generic",
                       [](const Account* _a) { return _a->type() != AccountType::INVESTMENT; },
                       100));
    }

    void LedgerControllerManager::registerLedger(LedgerControllerFactorySuper* _controller)
    {
        if (!_controller)
        {
            qDebug() << QObject::tr("This LedgerController is null.");
        }
        else if (m_controllers.contains(_controller->code))
        {
            qDebug() << QObject::tr("A LedgerController with this code (%1) is already registered.").arg(_controller->code);
        }
        else
        {
            m_controllers[_controller->code] = _controller;

            //Insert in the sorted list
            auto i = m_sorted.begin();
            for (; i != m_sorted.end(); ++i)
            {
                if ((*i)->defaultPriority >= _controller->defaultPriority)
                {
                    break;
                }
            }

            m_sorted.insert(i, _controller);
        }
    }

    LedgerController* LedgerControllerManager::ledgerFor(const Account* _account, QObject* _parent) const
    {
        if (_account->isPlaceholder())
        {
            return nullptr;
        }

        //Scan the list of controllers in order...
        for (LedgerControllerFactorySuper* f : m_sorted)
        {
            if (f->handles(_account))
            {
                return f->buildLedgerController(_account->ledger(), _parent);
            }
        }

        return nullptr;
    }

}
