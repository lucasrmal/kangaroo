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

#include "scriptengine.h"
#include "../interfaces/scriptable.h"

#include "../model/account.h"
#include "../model/currency.h"
#include "../model/institution.h"
#include "../model/ledger.h"
#include "../model/payee.h"
#include "../model/pricemanager.h"
#include "../model/properties.h"
#include "../model/security.h"
#include "../model/stored.h"
#include "../model/transaction.h"
#include "../model/transactionmanager.h"
#include "../model/modelexception.h"
#include "../model/picturemanager.h"

namespace KLib
{

    ScriptEngine* ScriptEngine::m_instance = nullptr;

    QString Locale::toNum(double _num) const
    {
        return m_locale.toString(_num, 'f', 2);
    }

    ScriptEngine::ScriptEngine()
    {
        registerScriptable(new ScriptableInstance2<Account, Account::getTopLevel>());
        registerScriptable(new ScriptableObject<Account>());
        registerScriptable(new ScriptableObject<Currency>());
        registerScriptable(new ScriptableInstance<CurrencyManager>());
        registerScriptable(new ScriptableObject<Institution>());
        registerScriptable(new ScriptableInstance<InstitutionManager>());
        registerScriptable(new ScriptableObject<Ledger>());
        registerScriptable(new ScriptableInstance<LedgerManager>());
        registerScriptable(new ScriptableObject<Payee>());
        registerScriptable(new ScriptableInstance<PayeeManager>());
        registerScriptable(new ScriptableObject<ExchangePair>());
        registerScriptable(new ScriptableInstance<PriceManager>());
        registerScriptable(new ScriptableObject<Properties>());
        registerScriptable(new ScriptableObject<Security>());
        registerScriptable(new ScriptableInstance<SecurityManager>());
        registerScriptable(new ScriptableObject<IStored>());
        registerScriptable(new ScriptableObject<Transaction>());
        registerScriptable(new ScriptableInstance<TransactionManager>());
        registerScriptable(new ScriptableStructure<Transaction::Split,
                                                     Transaction::Split::toScriptValue,
                                                     Transaction::Split::fromScriptValue>);
        registerScriptable(new ScriptableStructure<Amount,
                                                     Amount::toScriptValue,
                                                     Amount::fromScriptValue>);
        registerScriptable(new ScriptableInstance<PictureManager>());
        registerScriptable(new ScriptableStructure<Picture,
                                                     Picture::toScriptValue,
                                                     Picture::fromScriptValue>);
    }

    ScriptEngine* ScriptEngine::instance()
    {
        if (!m_instance)
            m_instance = new ScriptEngine();

        return m_instance;
    }

    bool ScriptEngine::execute(const QString& _script, QString& _output) const
    {
        QScriptEngine engine;

        QString txt;
        QScriptValue text = engine.newVariant(txt);
        engine.globalObject().setProperty("theval", text);
        QString printFunction = "function print(a) {theval += a; }\n";
        QString printLnFunction = "function println(a) {theval += a + \"\\n\"; }\n";

        for (Scriptable* s : m_instances)
        {
            s->addToEngine(&engine);
        }

        Locale l;
        QScriptValue localeVal = engine.newQObject(&l);
        engine.globalObject().setProperty("Locale", localeVal);

        QString script = printFunction + printLnFunction + _script;

        QScriptSyntaxCheckResult result = engine.checkSyntax(script);

        if (result.state() != QScriptSyntaxCheckResult::Valid)
        {
            _output = QString("%1:%2").arg(result.errorLineNumber()-2).arg(result.errorMessage());
            return false;
        }
        else
        {
            ModelException::beginScriptMode(&engine); //No throw exceptions, "throw" to the Script Engine context instead.
            engine.evaluate(script);
            ModelException::endScriptMode();

            if (engine.hasUncaughtException())
            {
                _output = QString("%1:%2")
                                    .arg(engine.uncaughtExceptionLineNumber()-2)
                                    .arg(engine.uncaughtException().toString())
                            + "\n\n" + QObject::tr("Backtrace:\n");

                for (QString s : engine.uncaughtExceptionBacktrace())
                {
                    _output += s + "\n";
                }

                return false;
            }
            else
            {
                _output = engine.globalObject().property("theval").toString();
                return true;
            }
        }
    }

    void ScriptEngine::registerScriptable(Scriptable* _s)
    {
        m_instances << _s;
    }

}
