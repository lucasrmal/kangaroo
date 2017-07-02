#include "secondarycurrencies.h"
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/currency.h>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>

using namespace KLib;

SecondaryCurrencies::SecondaryCurrencies(QWidget *parent) :
    AccountEditTab(parent)
{
    QLabel* lbl = new QLabel(tr("Select the additional currencies supported by your brokerage account."), this);
    m_listCurrencies = new QListWidget(this);

    lbl->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(lbl);
    layout->addWidget(m_listCurrencies);

    //Fill the currency list
    for (Currency* c : CurrencyManager::instance()->currencies())
    {
        QListWidgetItem* i = new QListWidgetItem(c->name(), m_listCurrencies);
        i->setData(Qt::UserRole, c->code());
        i->setFlags(i->flags() | Qt::ItemIsUserCheckable);
        i->setCheckState(Qt::Unchecked);

//        if (c.code == field("BookCurrency").toString())
//        {
//            i->setCheckState(Qt::Checked);
//            i->setFlags(0);
//        }
    }
}

bool SecondaryCurrencies::enableIf(int _accountType) const
{
    switch (_accountType)
    {
    case AccountType::BROKERAGE:
    case AccountType::INCOME:
    case AccountType::EXPENSE:
        return true;

    default:
        return false;
    }
}

void SecondaryCurrencies::fillData(const KLib::Account* _account)
{
    QSet<QString> currencies = _account->secondaryCurrencies();

    for (QListWidgetItem* i : m_listCurrencies->findItems("*", Qt::MatchWildcard))
    {
        if (currencies.contains(i->data(Qt::UserRole).toString()))
        {
            i->setCheckState(Qt::Checked);
        }
    }
}

void SecondaryCurrencies::save(KLib::Account* _account) const
{
    QSet<QString> currencies;

    for (QListWidgetItem* i : m_listCurrencies->findItems("*", Qt::MatchWildcard))
    {
        if (i->checkState() == Qt::Checked)
        {
            currencies.insert(i->data(Qt::UserRole).toString());
        }
    }

    _account->setSecondaryCurrencies(currencies);
}

QStringList SecondaryCurrencies::validate() const
{
    return {};
}
