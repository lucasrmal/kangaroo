#ifndef GNUCASHIMPORT_H
#define GNUCASHIMPORT_H

#include "../ImportExport/iimporter.h"
#include <KangarooLib/amount.h>
#include <QPair>
#include <QString>
#include <QDate>

class QXmlStreamReader;

namespace KLib
{
    enum class InvestmentAction;
}

using KLib::Amount;
using KLib::ErrorList;

struct GNUCashTags
{
    static const char* ROOT;
    static const char* FILEINFO;
    static const char* INSTITUTIONS;
    static const char* INSTITUTION;
    static const char* PAYEES;
    static const char* PAYEE;
    static const char* CURRENCIES;
    static const char* CURRENCY;
    static const char* SECURITIES;
    static const char* SECURITY;
    static const char* ACCOUNTS;
    static const char* ACCOUNT;
    static const char* PAIR;
    static const char* PRICES;
    static const char* PRICEPAIR;
    static const char* PRICE;
    static const char* TRANSACTIONS;
    static const char* TRANSACTION;
    static const char* SPLIT;
    static const char* SCHEDULES;
    static const char* BUDGETS;
};

class GnuCashImport : public KLib::IImporter
{
    public:
        GnuCashImport();

        QString fileType() const override;
        QString description() const override;
        QIcon   icon() const override;

        void import(const QString& _path, ErrorList& _errors) override;

    private:

};

#endif // GNUCASHIMPORT_H
