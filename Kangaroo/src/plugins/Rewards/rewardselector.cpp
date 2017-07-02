#include "rewardselector.h"
#include <KangarooLib/ui/widgets/amountedit.h>
#include <KangarooLib/model/currency.h>
#include <QComboBox>
#include <QHBoxLayout>

using namespace KLib;

RewardSelector::RewardSelector(const QList<RewardTier>& _tiers, const QString& _currency, QWidget *parent) :
    QWidget(parent),
    m_tiers(_tiers)
{
    m_selector = new QComboBox(this);
    m_txtAmount = new AmountEdit(this);

    m_selector->setMaximumWidth(15);
    m_txtAmount->setMinimum(-qInf());

    m_precision = CurrencyManager::instance()->get(_currency)->precision();
    m_txtAmount->setPrecision(m_precision);


    //Load the selector

    m_selector->addItem(tr("None"), Constants::NO_ID);

    for (RewardTier& t : m_tiers)
    {
        m_selector->addItem(t.name, t.id);
    }

    m_selector->addItem(tr("Custom"), Constants::NO_ID);
    onSelectorChanged(m_selector->currentIndex());

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addWidget(m_selector);
    layout->addWidget(m_txtAmount);

    connect(m_selector, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onSelectorChanged(int)));
}

void RewardSelector::setCurrent(int _idTier, const KLib::Amount& _custom)
{
    if (_idTier == Constants::NO_ID)
    {
        if (_custom == 0)
        {
            m_selector->setCurrentIndex(0);
        }
        else //Custom
        {
            m_selector->setCurrentIndex(m_selector->count()-1);
        }
    }
    else
    {
        m_selector->setCurrentIndex(m_selector->findData(_idTier));
    }
}

void RewardSelector::fromVariantList(const QList<QVariant>& _list)
{
    if (_list.count() > 0)
    {
        if (_list[0].toInt() != Constants::NO_ID)
        {
            setCurrent(_list[0].toInt(), 0);
        }
        else if (_list.count() == 2)
        {
            setCurrent(_list[0].toInt(), Amount::fromQVariant(_list[1]));
        }
        else
        {
            setCurrent(Constants::NO_ID, 0);
        }
    }
    else
    {
        setCurrent(Constants::NO_ID, 0);
    }
}

int RewardSelector::idTier() const
{
    return m_selector->currentData().toInt();
}

KLib::Amount RewardSelector::customAmount() const
{
    return m_selector->currentIndex() == m_selector->count()-1 ? m_txtAmount->amount()
                                                               : 0;
}

QList<QVariant> RewardSelector::toVariantList() const
{
    QList<QVariant> list;

    if (m_selector->currentIndex() == 0)
    {
        list << Constants::NO_ID << Amount(0).toQVariant();
    }
    else if (m_selector->currentIndex() == m_selector->count()-1)
    {
        list << Constants::NO_ID << m_txtAmount->amount().toQVariant();
    }
    else
    {
        list << m_selector->currentData();
    }

    return list;
}

void RewardSelector::setBaseAmount(const KLib::Amount& _amount)
{
    m_baseAmount = _amount;
    onSelectorChanged(m_selector->currentIndex());
}

void RewardSelector::onSelectorChanged(int _index)
{
    if (_index == 0) //None
    {
        m_txtAmount->setReadOnly(true);
        m_txtAmount->setAmount(0);
    }
    else if (_index == m_selector->count()-1) //Custom
    {
        m_txtAmount->setReadOnly(false);
        m_txtAmount->setAmount(0);
    }
    else //Tier
    {
        m_txtAmount->setReadOnly(true);
        m_txtAmount->setAmount(Amount((m_baseAmount * m_tiers[_index-1].rate) / 100.).toPrecision(m_precision));
    }
}
