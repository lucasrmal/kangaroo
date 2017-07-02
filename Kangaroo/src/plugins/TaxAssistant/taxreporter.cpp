#include "taxreporter.h"
#include "taxstatustab.h"

#include <KangarooLib/model/transaction.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/ledger.h>
#include <functional>

TaxReporter* TaxReporter::m_instance = nullptr;

using namespace KLib;


TaxReporter* TaxReporter::instance()
{
    if (!m_instance)
        m_instance = new TaxReporter();

    return m_instance;
}

TaxReport TaxReporter::taxReportFor(const QDate& _from, const QDate& _to) const
{
    TaxReport report;

    //Scan all the accounts, if income/expense and tax related, get list of transactions in the account for the period

    std::function<void(Account*, QList<TaxLineItem>&)> addTransactions;
    std::function<void(Account*)> findTaxAccounts;

    addTransactions = [_from, _to, &report, &addTransactions] (Account* a, QList<TaxLineItem>& list)
    {
        if (!a->isPlaceholder())
        {
            auto transactions = a->ledger()->transactionsBetween(_from, _to);

            for (Transaction* t : transactions)
            {
                list.append(TaxLineItem(a->id(), t));
            }
        }

        for (Account* child : a->getChildren())
        {
            addTransactions(child, list);
        }
    };

    findTaxAccounts = [&report, &findTaxAccounts, &addTransactions] (Account* a)
    {
        switch (a->type())
        {
        case AccountType::INCOME:
        case AccountType::EXPENSE:
        case AccountType::TOPLEVEL:
            if (a->properties()->contains(TaxStatusTab::TAX_ITEM_PROPERTY)
                && a->type() != AccountType::TOPLEVEL)
            {
                addTransactions(a, report[a->properties()->get(TaxStatusTab::TAX_ITEM_PROPERTY).toString()]);
            }
            else
            {
                for (Account* child : a->getChildren())
                {
                    findTaxAccounts(child);
                }
            }
            break;

        default:
            return;
        }
    };

    findTaxAccounts(Account::getTopLevel());

    //Remove empty categories
    auto i = report.begin();

    while (i != report.end())
    {
        if (i->isEmpty())
        {
            i = report.erase(i);
        }
        else
        {
            ++i;
        }
    }

    //Now we want to sort the transactions in each list by date
    for (QList<TaxLineItem>& list : report)
    {
        std::sort(list.begin(), list.end(), [](const TaxLineItem& a, const TaxLineItem& b) {
            return a.second->date() < b.second->date();
        });
    }

    return report;
}


