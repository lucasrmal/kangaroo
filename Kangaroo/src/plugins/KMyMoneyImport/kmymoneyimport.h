#ifndef KMYMONEYIMPORT_H
#define KMYMONEYIMPORT_H

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

struct KMMTags
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

struct KMMSecurity
{
    QString id;
    int type;
    QString name;
    QString market;
    QString symbol;
    QString currency;
    int prec;
};

struct KMMAccount
{
    QString id;
    int type;
    QString name;
    QString currency;
    QString institution;
    QString code;
    QString description;
    QString parent;
    bool open;
};

struct KMMSplit
{
    QString memo;
    QString account;
    Amount amount;
    QString action;
    Amount price;
};

struct KMMTransaction
{
    KMMTransaction() ;

    QDate date;
    QString payee;
    QString number;
    QString memo;
    QString frac;
    KLib::InvestmentAction inv_action;

    QList<KMMSplit> splits;
};

class KMyMoneyImport : public KLib::IImporter
{
    public:
        explicit KMyMoneyImport();

        QString fileType() const override;
        QString description() const override;
        QIcon   icon() const override;

        void import(const QString& _path, ErrorList& _errors) override;

    private:
        void loadAccounts(QXmlStreamReader& _reader, ErrorList& _errors);
        void loadPayees(QXmlStreamReader& _reader, ErrorList& _errors);
        void loadInstitutions(QXmlStreamReader& _reader, ErrorList& _errors);
        void loadCurrencies(QXmlStreamReader& _reader, ErrorList& _errors);
        void loadSecurities(QXmlStreamReader& _reader, ErrorList& _errors);
        void loadPrices(QXmlStreamReader& _reader, ErrorList& _errors);
        void loadTransactions(QXmlStreamReader& _reader, ErrorList& _errors);

        void insertAccounts(ErrorList& _errorsZ);
        void insertAccount(QString _kid, ErrorList& _errors);
        void insertPrices(ErrorList& _errors);
        void insertTransactions(ErrorList& _errors);
        void insertSecurities(ErrorList& _errors);

        QHash<QString, int> m_accountIds;
        QHash<QString, KMMAccount> m_kmmAccounts;
        QHash<QString, int> m_payees;
        QHash<QString, int> m_institutions;
        QHash<QString, int> m_securities;
        QList<KMMSecurity> m_kmmSecurities;
        QList<KMMTransaction> m_kmmTransactions;

        QHash<QPair<QString, QString>, QList<QPair<QString, QDate> > > m_prices;

};

#endif // KMYMONEYIMPORT_H
