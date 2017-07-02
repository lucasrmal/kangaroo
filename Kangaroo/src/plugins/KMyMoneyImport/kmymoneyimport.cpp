#include "kmymoneyimport.h"
#include <KangarooLib/ui/core.h>
#include <KangarooLib/controller/io.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/payee.h>
#include <KangarooLib/model/institution.h>
#include <KangarooLib/model/transactionmanager.h>
#include <KangarooLib/model/investmenttransaction.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/security.h>
#include <KangarooLib/model/pricemanager.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/model/ledger.h>

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QApplication>

using namespace KLib;

const char* KMMTags::ROOT = "KMYMONEY-FILE";
const char* KMMTags::FILEINFO = "FILEINFO";
const char* KMMTags::INSTITUTIONS = "INSTITUTIONS";
const char* KMMTags::INSTITUTION = "INSTITUTION";
const char* KMMTags::PAYEES = "PAYEES";
const char* KMMTags::PAYEE = "PAYEE";
const char* KMMTags::CURRENCIES = "CURRENCIES";
const char* KMMTags::CURRENCY = "CURRENCY";
const char* KMMTags::SECURITIES = "SECURITIES";
const char* KMMTags::SECURITY = "SECURITY";
const char* KMMTags::ACCOUNTS = "ACCOUNTS";
const char* KMMTags::ACCOUNT = "ACCOUNT";
const char* KMMTags::PAIR = "PAIR";
const char* KMMTags::PRICES = "PRICES";
const char* KMMTags::PRICEPAIR = "PRICEPAIR";
const char* KMMTags::PRICE = "PRICE";
const char* KMMTags::TRANSACTIONS = "TRANSACTIONS";
const char* KMMTags::TRANSACTION = "TRANSACTION";
const char* KMMTags::SPLIT = "SPLIT";
const char* KMMTags::SCHEDULES = "SCHEDULES";
const char* KMMTags::BUDGETS = "BUDGETS";

KMMTransaction::KMMTransaction() :
    inv_action(InvestmentAction::Invalid)
{
}

KMyMoneyImport::KMyMoneyImport()
{
}

QString KMyMoneyImport::fileType() const
{
    return QObject::tr("KMyMoney XML file") + " (*.xml)";
}

QString KMyMoneyImport::description() const
{
    return QObject::tr("KMyMoney XML file");
}

QIcon KMyMoneyImport::icon() const
{
    return Core::icon("filekmy");
}

void KMyMoneyImport::import(const QString& _path, ErrorList& _errors)
{
    /* Cursor */
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    try
    {
        /* Clear the old stuff */
        m_accountIds.clear();
        m_kmmAccounts.clear();
        m_payees.clear();
        m_institutions.clear();
        m_securities.clear();
        m_prices.clear();

        /* Import the file */

        QFile* file = new QFile(_path);
        QString baseCurrency;

        /* If we can't open it, let's show an error message. */
        if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QApplication::restoreOverrideCursor();
            _errors << Error(Critical, QObject::tr("Unable to open file: %1").arg(file->errorString()));
            return;
        }

        QXmlStreamReader xml(file);

        if (xml.readNextStartElement())
        {
            if (xml.name() != KMMTags::ROOT)
            {
                QApplication::restoreOverrideCursor();
                _errors << Error(Critical,
                                 QObject::tr("This file is not a valid XML KMyMoney file. The root tag is %1")
                                    .arg(xml.name().toString()));
                return;
            }

            while(!xml.atEnd() && !xml.hasError())
            {
               if (xml.tokenType() == QXmlStreamReader::StartElement)
               {
                   if (xml.name() == KMMTags::ACCOUNTS)
                   {
                       loadAccounts(xml, _errors);
                   }
                   else if (xml.name() == KMMTags::PAYEES)
                   {
                       loadPayees(xml, _errors);
                   }
                   else if (xml.name() == KMMTags::INSTITUTIONS)
                   {
                       loadInstitutions(xml, _errors);
                   }
                   else if (xml.name() == KMMTags::CURRENCIES)
                   {
                       loadCurrencies(xml, _errors);
                   }
                   else if (xml.name() == KMMTags::SECURITIES)
                   {
                       loadSecurities(xml, _errors);
                   }
                   else if (xml.name() == KMMTags::PRICES)
                   {
                       loadPrices(xml, _errors);
                   }
                   else if (xml.name() == KMMTags::TRANSACTIONS)
                   {
                       loadTransactions(xml, _errors);
                   }
                   else if (xml.name() == KMMTags::SCHEDULES)
                   {
                       _errors << Error(Warning, QObject::tr("Import of KMyMoney schedules is not supported (yet!)."));
                       xml.readNext();
                   }
                   else if (xml.name() == KMMTags::BUDGETS)
                   {
                       _errors << Error(Warning, QObject::tr("Import of KMyMoney budgets is not supported (yet!)."));
                       xml.readNext();
                   }
                   else if (xml.name() == KMMTags::PAIR)
                   {
                       QXmlStreamAttributes attributes = xml.attributes();

                       QString key = IO::getAttribute("key", attributes);
                       QString value = IO::getAttribute("value", attributes);

                       if (key == "kmm-baseCurrency")
                       {
                           baseCurrency = value;
                       }
                       xml.readNext();
                   }
                   else
                   {
                       xml.readNext();
                   }
               }
               else
               {
                   xml.readNext();
               }
            }

        }

        /* Error handling. */
        QString error;
        if(xml.hasError())
        {
           error == xml.errorString();
        }

        xml.clear();
        file->close();
        delete file;

        // Finish loading
        insertSecurities(_errors);
        insertAccounts(_errors);
        insertPrices(_errors);
        insertTransactions(_errors);

        if (!baseCurrency.isEmpty())
            Account::getTopLevel()->setCurrency(baseCurrency);

        if (!error.isEmpty())
        {
            QApplication::restoreOverrideCursor();
            _errors << Error(Critical,
                             QObject::tr("An error occured while parsing the file: %1").arg(xml.errorString()));
            return;
        }

    }
    catch (ModelException e)
    {
        _errors << Error(Critical,
                         QObject::tr("Invalid data discovered in the file: %1").arg(e.description()));
    }

    catch (IOException e)
    {
        _errors << Error(Critical,
                         QObject::tr("An error occured while parsing the file: %1").arg(e.description()));
    }

    /* Cursor */
    QApplication::restoreOverrideCursor();
}

void KMyMoneyImport::loadAccounts(QXmlStreamReader& _reader, ErrorList&)
{
    KMMAccount a;

    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == KMMTags::ACCOUNTS))
    {
        if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == KMMTags::ACCOUNT)
        {
            QXmlStreamAttributes attributes = _reader.attributes();

            a.id = IO::getAttribute("id", attributes);
            a.type = IO::getAttribute("type", attributes).toInt();
            a.name = IO::getAttribute("name", attributes);
            a.currency = IO::getAttribute("currency", attributes);
            a.code = IO::getAttribute("number", attributes);
            a.institution = IO::getAttribute("institution", attributes);
            a.description = IO::getAttribute("description", attributes);
            a.parent = IO::getAttribute("parentaccount", attributes);
            a.open = true;

            m_kmmAccounts.insert(a.id, a);
        }
        else if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == KMMTags::PAIR)
        {
            QXmlStreamAttributes attributes = _reader.attributes();

            if (attributes.hasAttribute("key") && attributes.hasAttribute("value") &&
                attributes.value("key") == "mm-closed" && attributes.value("value") == "yes")
            {
                a.open = false;
                m_kmmAccounts[a.id] = a;
            }
        }

        _reader.readNext();
    }
}

void KMyMoneyImport::insertAccount(QString _kid, ErrorList& _errors)
{
    if (!m_kmmAccounts.contains(_kid))
    {
        throw IOException(QObject::tr("A parent account is missing in the file."));
    }

    KMMAccount a = m_kmmAccounts[_kid];
    Account* parent;

    if (a.parent == "")
    {
        parent = Account::getTopLevel();
    }
    else
    {
        if (!m_accountIds.contains(a.parent))
        {
            insertAccount(a.parent, _errors);
        }

        parent = Account::getTopLevel()->getChild(m_accountIds[a.parent]);
    }

    if (!a.institution.isEmpty() && !m_institutions.contains(a.institution))
    {
        throw IOException(QObject::tr("An institution is missing in the file."));
    }

    int institution = a.institution.isEmpty() ? Constants::NO_ID : m_institutions[a.institution];
    int security = Constants::NO_ID;

    switch (a.type)
    {
    case 0:     //UnknownAccountType, /**< For error handling */
        throw IOException(QObject::tr("UnknownAccountType in the file, which is currently unsupported."));

    case 1:     //Checkings,            /**< Standard checking account */
        a.type = AccountType::CHECKING;
        break;

    case 2:     //Savings,              /**< Typical savings account */
        a.type = AccountType::SAVINGS;
        break;

    case 3:     //Cash,                 /**< Denotes a shoe-box or pillowcase stuffed with cash */
        a.type = AccountType::CASH;
        break;

    case 4:     //CreditCard,           /**< Credit card accounts */
        a.type = AccountType::CREDITCARD;
        break;

    case 5:     //Loan,                 /**< Loan and mortgage accounts (liability) */
        a.type = AccountType::LIABILITY;
        break;

    case 6:     //CertificateDep,       /**< Certificates of Deposit */
        a.type = AccountType::SAVINGS;
        break;

    case 7:     //Investment (brokerage), /**< Investment account */
        a.type = AccountType::BROKERAGE;
        break;

    case 8:     //MoneyMarket,          /**< Money Market Account */
        a.type = AccountType::SAVINGS;
        break;

    case 9:     //Asset,                /**< Denotes a generic asset account.*/
        a.type = AccountType::ASSET;
        break;

    case 10:    //Liability,            /**< Denotes a generic liability account.*/
        a.type = AccountType::LIABILITY;
        break;

    case 11:    //Currency,             /**< Denotes a currency trading account. */
        throw IOException(QObject::tr("Currency Trading Account in the file, which is currently unsupported."));
        break;

    case 12:    //Income,               /**< Denotes an income account */
        a.type = AccountType::INCOME;
        break;

    case 13:    //Expense,              /**< Denotes an expense account */
        a.type = AccountType::EXPENSE;
        break;

    case 14:    //AssetLoan,            /**< Denotes a loan (asset of the owner of this object) */
        a.type = AccountType::ASSET;
        break;

    case 15:    //Stock,                /**< Denotes an security account as sub-account for an investment */
        a.type = AccountType::INVESTMENT;
        break;

    case 16:    //Equity,               /**< Denotes an equity account e.g. opening/closeing balance */
        a.type = AccountType::EQUITY;
        break;

    }

    if (a.type == AccountType::INVESTMENT)
    {
        security = m_securities[a.currency];
        a.currency = "";
    }

    try
    {
        int id = parent->addChild(a.name,
                                  a.type,
                                  a.currency,
                                  security,
                                  false,
                                  institution,
                                  a.code,
                                  a.description)->id();

        if (!a.open)
            parent->getChild(id)->setOpen(false);

        m_accountIds[a.id] = id;
        m_kmmAccounts.remove(_kid);
    }
    catch (ModelException)
    {
        throw IOException(QObject::tr("An account definition is invalid in the file."));
    }
}

void KMyMoneyImport::insertAccounts(ErrorList& _errors)
{
    while (!m_kmmAccounts.isEmpty())
    {
        insertAccount(m_kmmAccounts.begin().key(), _errors);
    }
}

void KMyMoneyImport::loadPayees(QXmlStreamReader& _reader, ErrorList& _errors)
{
    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == KMMTags::PAYEES))
    {
        if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == KMMTags::PAYEE)
        {
            try
            {
                QXmlStreamAttributes attributes = _reader.attributes();

                QString id = IO::getAttribute("id", attributes);
                QString name = IO::getAttribute("name", attributes);

                m_payees.insert(id, PayeeManager::instance()->add(name)->id());
            }
            catch (IOException)
            {
                _errors << Error(Warning, QObject::tr("Unable to load a payee due to missing argument."));
            }
        }

        _reader.readNext();
    }
}

void KMyMoneyImport::loadCurrencies(QXmlStreamReader& _reader, ErrorList& _errors)
{
    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == KMMTags::CURRENCIES))
    {
        if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == KMMTags::CURRENCY)
        {
            try
            {
                QXmlStreamAttributes attributes = _reader.attributes();

                QString id = IO::getAttribute("id", attributes);
                QString symbol = IO::getAttribute("symbol", attributes);
                QString name = IO::getAttribute("name", attributes);
                double saf = IO::getOptAttribute("saf", attributes, 100).toDouble();

                int prec = (int) ceil(log10(saf)); // Convert fraction to num of digits. Ex: 100 => 2, 500 => ceil(2.69...)=3

                try
                {
                    CurrencyManager::instance()->add(id, name, symbol, prec);
                }
                catch (ModelException)
                {
                    CurrencyManager::instance()->get(id)->setName(name);
                    CurrencyManager::instance()->get(id)->setCustomSymbol(symbol);
                    CurrencyManager::instance()->get(id)->setPrecision(prec);
                }
            }
            catch (IOException)
            {
                _errors << Error(Warning, QObject::tr("Unable to load a currency due to missing argument."));
            }
        }

        _reader.readNext();
    }
}

void KMyMoneyImport::loadInstitutions(QXmlStreamReader& _reader, ErrorList& _errors)
{
    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == KMMTags::INSTITUTIONS))
    {
        if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == KMMTags::INSTITUTION)
        {
            try
            {
                QXmlStreamAttributes attributes = _reader.attributes();

                QString id = IO::getAttribute("id", attributes);
                QString name = IO::getAttribute("name", attributes);

                m_institutions.insert(id, InstitutionManager::instance()->add(name)->id());
            }
            catch (IOException)
            {
                _errors << Error(Warning, QObject::tr("Unable to load an institution due to missing argument."));
            }
        }

        _reader.readNext();
    }
}

void KMyMoneyImport::loadSecurities(QXmlStreamReader& _reader, ErrorList& _errors)
{
    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == KMMTags::SECURITIES))
    {
        if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == KMMTags::SECURITY)
        {
            try
            {
                QXmlStreamAttributes attributes = _reader.attributes();
                KMMSecurity s;

                s.id = IO::getAttribute("id", attributes);
                s.type = IO::getAttribute("type", attributes).toInt();
                s.name = IO::getAttribute("name", attributes);
                s.market = IO::getAttribute("trading-market", attributes);
                s.symbol = IO::getAttribute("symbol", attributes);
                s.currency = IO::getAttribute("trading-currency", attributes);
                double saf = IO::getOptAttribute("saf", attributes, 100).toDouble();

                s.prec = (int) ceil(log10(saf)); // Convert denominator to num of digits. Ex: 100 => 2, 500 => ceil(2.69...)=3

                /*
                 * KMM Types:
                 * 0: Stock
                 * 1: Mutual Fund
                 * 2: Bond
                 * 3: Currency
                 * 4: None
                 *
                 * We ignore 3 and 4.
                 */
                switch (s.type)
                {
                case 0:
                    s.type = (int) SecurityType::Stock;
                    break;
                case 1:
                    s.type = (int) SecurityType::MutualFund;
                    break;
                case 2:
                    s.type = (int) SecurityType::Bond;
                    break;
                default:
                    _errors << Error(Warning, QObject::tr("Unsupported security type: %1").arg(s.type));
                    _reader.readNext();
                    continue;
                }

                m_kmmSecurities << s;
            }
            catch (IOException)
            {
                _errors << Error(Warning, QObject::tr("Unable to load a security due to missing argument."));
            }
        }

        _reader.readNext();
    }
}

void KMyMoneyImport::insertSecurities(ErrorList& _errors)
{
    for (KMMSecurity s : m_kmmSecurities)
    {
        try
        {
            m_securities.insert(s.id,
                                SecurityManager::instance()->add((SecurityType) s.type,
                                                                 s.name,
                                                                 s.symbol,
                                                                 s.market,
                                                                 "",
                                                                 s.currency,
                                                                 s.prec)->id());
        }
        catch (ModelException e)
        {
            _errors << Error(Warning, QObject::tr("Unable to insert security %1: %2.")
                             .arg(s.name)
                             .arg(e.description()));
        }
    }
}

void KMyMoneyImport::loadPrices(QXmlStreamReader& _reader, ErrorList& _errors)
{
    QString from, to;
    QList<QPair<QString, QDate> > prices;

    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == KMMTags::PRICES))
    {
        try
        {
            if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == KMMTags::PRICEPAIR)
            {
                QXmlStreamAttributes attributes = _reader.attributes();

                from = IO::getAttribute("from", attributes);
                to   = IO::getAttribute("to", attributes);
            }
            else if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == KMMTags::PRICE)
            {
                QXmlStreamAttributes attributes = _reader.attributes();

                QString price = IO::getAttribute("price", attributes);
                QDate date = QDate::fromString(IO::getAttribute("date", attributes), Qt::ISODate);

                prices << QPair<QString, QDate>(price, date);
            }
            else if (_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == KMMTags::PRICEPAIR)
            {
                m_prices[QPair<QString, QString>(from, to)] = prices;
                prices.clear();
            }
        }
        catch (IOException e)
        {
            _errors << Error(Warning, QObject::tr("Unable to load a price due to missing argument: %1.").arg(e.description()));
        }

        _reader.readNext();
    }
}

void KMyMoneyImport::insertPrices(ErrorList&)
{
    for (auto i = m_prices.begin(); i != m_prices.end(); ++i)
    {
        QString from;

        if (i.key().first.length() == 3) // CC
        {
            from = i.key().first;
        }
        else
        {
            from = PriceManager::securityId(m_securities[i.key().first]);
        }

        ExchangePair* p = PriceManager::instance()->getOrAdd(from, i.key().second);

        for (auto j = i.value().begin(); j != i.value().end(); ++j)
        {
            QString first,second;
            double value;

            first = (*j).first.section('/', 0,0);
            second = (*j).first.section('/',1,1);

            if (!second.isEmpty())
            {
                value = first.toDouble() / second.toDouble();
            }
            else
            {
                value = first.toDouble();
            }
            p->set((*j).second, value);
        }
    }
}

void KMyMoneyImport::loadTransactions(QXmlStreamReader& _reader, ErrorList& _errors)
{
    KMMTransaction t;

    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == KMMTags::TRANSACTIONS))
    {
        if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == KMMTags::TRANSACTION)
        {
            QXmlStreamAttributes attributes = _reader.attributes();

            t.date = QDate::fromString(IO::getAttribute("postdate", attributes), Qt::ISODate);
            t.memo = IO::getAttribute("memo", attributes);
        }
        else if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == KMMTags::SPLIT)
        {
            QXmlStreamAttributes attributes = _reader.attributes();
            KMMSplit s;

            s.amount = Amount::fromStoreable2(IO::getAttribute("shares", attributes));
            s.account = IO::getAttribute("account", attributes);
            s.memo = IO::getAttribute("memo", attributes);
            s.action = IO::getOptAttribute("action", attributes);
            s.price = Amount::fromStoreable2(IO::getOptAttribute("price", attributes, "0/1"));

            if (!s.action.isEmpty())
            {
                if (s.action.toLower() == "buy")
                {
                    t.inv_action = s.amount > 0 ? InvestmentAction::Buy
                                                : InvestmentAction::Sell;
                }
                else if (s.action.toLower() == "add")
                {
                    t.inv_action = InvestmentAction::Transfer;
                }
                else if (s.action.toLower() == "reinvest")
                {
                    t.inv_action = InvestmentAction::ReinvestDiv;
                }
                else if (s.action.toLower() == "split")
                {
                    t.frac = IO::getAttribute("shares", attributes);
                    t.inv_action = InvestmentAction::StockSplit;
                }
                else if (s.action.toLower() == "dividend"
                         || s.action.toLower() == "intincome")
                {
                    t.inv_action = InvestmentAction::Dividend;
                }
                else
                {
                    _errors << Error(Warning, QObject::tr("Unknown split action: %1. Skipping transaction %2.")
                                     .arg(s.action).arg(t.number));

                    while (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == KMMTags::SPLIT)
                        _reader.readNext();
                    continue;
                }
            }

            if (t.memo.isEmpty() && !s.memo.isEmpty())
                t.memo = s.memo;

            if (t.payee.isEmpty())
                t.payee = IO::getAttribute("payee", attributes);

            if (t.number.isEmpty())
                t.number = IO::getAttribute("number", attributes);

            t.splits << s;
        }
        else if (_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == KMMTags::TRANSACTION)
        {
            m_kmmTransactions << t;
            t = KMMTransaction();
        }

        _reader.readNext();
    }
}

void KMyMoneyImport::insertTransactions(ErrorList& _errors)
{
    auto accountAction = [] (int idAccount)
    {
        return Account::getTopLevel()->account(idAccount)->type();
    };

    auto accountCurrency = [] (int idAccount)
    {
        return Account::getTopLevel()->account(idAccount)->mainCurrency();
    };

    try
    {
        for (KMMTransaction t : m_kmmTransactions)
        {
            QList<Transaction::Split> splits;
            QList<InvestmentSplitType> types;
            SplitFraction frac;
            Amount pricePerShare;
            int idInvAccount = Constants::NO_ID;

            try
            {
                //Look at the splits, if investment transaction, find correct actions.
                for (KMMSplit s : t.splits)
                {
                    Account* sa = Account::getTopLevel()->getChild(m_accountIds[s.account]);

                    if (t.inv_action != InvestmentAction::Invalid)
                    {
                        if (!s.action.isEmpty())
                        {
                            if (s.action.toLower() == "buy")
                            {
                                pricePerShare = s.price;
                                types << InvestmentSplitType::Investment;

                                if (s.amount < 0)
                                     s.action = "Sell";

                            }
                            else if (s.action.toLower() == "add")
                            {
                                //We add two types since the second split will be added right after adding this split.
                                if (s.amount < 0) //Remove
                                {
                                    types << InvestmentSplitType::InvestmentFrom;
                                    types << InvestmentSplitType::InvestmentTo;
                                }
                                else // Add
                                {
                                    types << InvestmentSplitType::InvestmentTo;
                                    types << InvestmentSplitType::InvestmentFrom;
                                }
                            }
                            else if (s.action.toLower() == "reinvest")
                            {
                                pricePerShare = s.price;
                                types << InvestmentSplitType::Investment;

                            }
                            else if (s.action.toLower() == "dividend"
                                     || s.action.toLower() == "intincome")
                            {
                                //Anchor split
                                idInvAccount = m_accountIds[s.account];
                            }
                            else if (s.action.toLower() == "split")
                            {
                                //Anchor split
                                idInvAccount = m_accountIds[s.account];

                                QStringList fracList = t.frac.split("/");

                                if (fracList.size() == 2)
                                {
                                    frac = SplitFraction(fracList[0].toInt(), fracList[1].toInt());
                                }
                                else
                                {
                                    ModelException::throwException(QObject::tr("Wrong split fraction in split. Skipping transaction."),
                                                                   nullptr);
                                }

                            }

                            if (t.memo.isEmpty())
                                t.memo = QString("%1 (%2)").arg(s.action).arg(sa->name());
                        }
                        else // (s.action.isEmpty())
                        {
                            if (accountAction(m_accountIds[s.account]) == AccountType::EXPENSE) //Fee
                            {
                                types << InvestmentSplitType::Fee;
                            }
                            else if ((t.inv_action == InvestmentAction::Dividend
                                      || t.inv_action == InvestmentAction::ReinvestDiv)
                                     && accountAction(m_accountIds[s.account]) == AccountType::INCOME) //Dividend source
                            {
                                types << InvestmentSplitType::DistributionSource;
                            }
                            else if (t.inv_action == InvestmentAction::Dividend)
                            {
                                types << InvestmentSplitType::DistributionDest;
                            }
                            else if (t.inv_action == InvestmentAction::ReinvestDiv)
                            {
                                types << InvestmentSplitType::CashInLieu;
                            }
                            else if (t.inv_action == InvestmentAction::Buy
                                     || t.inv_action == InvestmentAction::Sell) //CostProceeds
                            {
                                types << InvestmentSplitType::CostProceeds;
                            }
                        }
                    }

                    if (s.amount != 0) //Do not add anchor splits...
                    {
                        splits << Transaction::Split(s.amount,
                                                     m_accountIds[s.account],
                                                     accountCurrency(m_accountIds[s.account]),
                                                     s.memo);


                        if (t.inv_action == InvestmentAction::Transfer) //Add or remove...
                        {
                            int idTrading = SecurityManager::instance()->get(sa->idSecurity())->tradingAccount()->id();
                            splits << Transaction::Split(-s.amount,
                                                         idTrading,
                                                         "");
                        }
                    }
                }

                //Add the trading splits
                Transaction::addTradingSplits(splits);

                //Now add the transaction
                if (t.inv_action == InvestmentAction::Invalid) //Regular transaction
                {
                    Transaction* trans = new Transaction();
                    trans->setDate(t.date);
                    trans->setNo(t.number);
                    trans->setIdPayee(t.payee.isEmpty() ? Constants::NO_ID
                                                        : m_payees[t.payee]);
                    trans->setMemo(t.memo);
                    trans->setSplits(splits);

                    LedgerManager::instance()->addTransaction(trans);
                }
                else
                {

                    //Add trading actions
                    while (types.count() < splits.count())
                    {
                        types << InvestmentSplitType::Trading;
                    }

                    InvestmentTransaction* tr = new InvestmentTransaction();

                    tr->setDate(t.date);
                    tr->setNo(t.number);
                    if (!t.payee.isEmpty()) tr->setIdPayee(m_payees[t.payee]);
                    tr->setMemo(t.memo);

                    switch (t.inv_action)
                    {
                    case InvestmentAction::Buy:
                    case InvestmentAction::Sell:
                        tr->makeBuySellFee(t.inv_action,
                                        pricePerShare,
                                        splits,
                                        types);
                        break;

                    case InvestmentAction::ReinvestDiv:
                        tr->makeReinvestedDivDist(t.inv_action,
                                                  pricePerShare,
                                                  splits,
                                                  types);
                        break;

                    case InvestmentAction::Dividend:
                        tr->makeDivDist(t.inv_action,
                                        idInvAccount,
                                        splits,
                                        types);
                        break;

                    case InvestmentAction::StockSplit:
                        tr->makeSplit(idInvAccount,
                                      frac);
                        break;

                    case InvestmentAction::Transfer:
                        tr->makeTransferSwap(t.inv_action,
                                             splits,
                                             types);
                        break;

                    default:
                        break;
                    }

                    LedgerManager::instance()->addTransaction(tr);
                }
            }
            catch (ModelException e)
            {
                _errors << Error(Warning, QObject::tr("Unable to add transaction %1 on %2: %3")
                                 .arg(t.memo)
                                 .arg(t.date.toString())
                                 .arg(e.description()));
            }
        }
    }
    catch (ModelException)
    {
        throw IOException(QObject::tr("A transaction definition is invalid in the file."));
    }
}






