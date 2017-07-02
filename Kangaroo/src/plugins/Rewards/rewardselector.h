#ifndef REWARDSELECTOR_H
#define REWARDSELECTOR_H

#include <QWidget>
#include <KangarooLib/amount.h>
#include "rewardsprogram.h"

class QComboBox;

namespace KLib
{
    class AmountEdit;
}

class RewardSelector : public QWidget
{
    Q_OBJECT

    public:
        explicit RewardSelector(const QList<RewardTier>& _tiers, const QString& _currency, QWidget *parent = 0);

        void setCurrent(int _idTier, const KLib::Amount& _custom);
        void setBaseAmount(const KLib::Amount& _amount);
        void fromVariantList(const QList<QVariant>& _list);

        int idTier() const;
        KLib::Amount customAmount() const;

        QList<QVariant> toVariantList() const;

    signals:

    private slots:

        void onSelectorChanged(int _index);

    private:
        KLib::AmountEdit*   m_txtAmount;
        QComboBox*          m_selector;

        QList<RewardTier>   m_tiers;
        KLib::Amount        m_baseAmount;

        int                 m_precision;

};

#endif // REWARDSELECTOR_H
