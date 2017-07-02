#ifndef REWARDSLEDGERCONTROLLER_H
#define REWARDSLEDGERCONTROLLER_H

#include <KangarooLib/controller/ledger/genericledgercontroller.h>

namespace RewardsLedgerColumn
{
    enum Columns
    {
        STATUS = 0,
        FLAG,
        DATE,
        NO,
        MEMO,
        PAYEE,
        CLEARED,
        TRANSFER,
        REWARD,
        DEBIT,
        CREDIT,
        BALANCE,
        NumColumns
    };
}

namespace KLib
{
    class Ledger;
    class Account;
}

class RewardsProgram;
class RewardsLedgerController;

class RewardsLedgerBuffer : public KLib::LedgerBuffer
{
    public:
        int idTier;
        KLib::Amount rewardAmount;

        KLib::Amount computeRewards(const RewardsLedgerController* _controller) const;

        void clear() override;

        bool setData(int _column, int _row, const QVariant& _value, KLib::LedgerController* _controller) override;
        QVariant data(int _column, int _row, bool _editRole, const KLib::LedgerController* _controller) const override;

        QStringList validate(int& _firstErrorColumn, const KLib::LedgerController* _controller);

        QList<KLib::Transaction::Split> splitsForSaving(KLib::LedgerController* _controller, bool& _ok) const override;

        static KLib::Amount removeRewardSplits(QList<KLib::Transaction::Split>& _splits,
                                               const RewardsLedgerController* _controller);

        static int tierIdForAmount(const KLib::Amount& _rewardAmount,
                                   const KLib::Amount& _transactionAmount,
                                   const RewardsLedgerController* _controller);

    protected:
        void loadSplits(const QList<KLib::Transaction::Split>& _splits, const KLib::LedgerController* _controller) override;
};

class RewardsLedgerController : public KLib::GenericLedgerController
{
    Q_OBJECT

    public:
        explicit RewardsLedgerController(KLib::Ledger* _ledger, QObject *_parent = 0);

        int columnCount(const QModelIndex& _parent = QModelIndex()) const;

        int col_status() const override      { return RewardsLedgerColumn::STATUS; }
        int col_flag() const override        { return RewardsLedgerColumn::FLAG; }
        int col_no() const override          { return RewardsLedgerColumn::NO; }
        int col_date() const override        { return RewardsLedgerColumn::DATE; }
        int col_memo() const override        { return RewardsLedgerColumn::MEMO; }
        int col_payee() const override       { return RewardsLedgerColumn::PAYEE; }
        int col_cleared() const override     { return RewardsLedgerColumn::CLEARED; }
        int col_transfer() const override    { return RewardsLedgerColumn::TRANSFER; }
        int col_debit() const override       { return RewardsLedgerColumn::DEBIT; }
        int col_credit() const override      { return RewardsLedgerColumn::CREDIT; }
        int col_balance() const override     { return RewardsLedgerColumn::BALANCE; }

        RewardsProgram* rewardsProgram() const          { return m_program; }
        int             idRewardsFromAccount() const    { return m_idRewardsFrom; }
        int             idRewardsToAccount() const      { return m_idRewardsTo; }
        const QString&  rewardCurrency() const          { return m_rewardCurrency; }

        bool alignRight(int _column) const override { return _column >= RewardsLedgerColumn::REWARD; }

        KLib::LedgerWidgetDelegate* buildDelegate(KLib::LedgerWidget* _widget) const override;

        QVariant headerData(int _section,
                            Qt::Orientation _orientation,
                            int _role = Qt::DisplayRole) const override;

        QVariant cacheData(int _column, int _cacheRow, int _row, bool _editRole) const override;
        QList<KLib::Transaction::Split> displayedSplits(const KLib::Transaction* _tr) const override;

        KLib::Amount totalChargedForReward(const QModelIndex& _index) const;

    private slots:
        void onAccountModified(KLib::Account* _a);
        void onAccountRemoved(KLib::Account* _a);
        void onRewardsProgramRemoved(RewardsProgram* _p);

    private:
        void resetRewardsProgramInfo();

        RewardsProgram* m_program;
        int m_idRewardsFrom;
        int m_idRewardsTo;
        QString m_rewardCurrency;

};

class RewardsLedgerWidgetDelegate : public KLib::LedgerWidgetDelegate
{
    Q_OBJECT

    public:
        RewardsLedgerWidgetDelegate(const KLib::LedgerController* _controller, KLib::LedgerWidget* _widget);

        QWidget* createEditor(QWidget* _parent,
                              const QStyleOptionViewItem& _option,
                              const QModelIndex& _index) const override;

        void setEditorData(QWidget* _editor,
                           const QModelIndex& _index) const override;

        void setModelData(QWidget* _editor,
                          QAbstractItemModel* _model,
                          const QModelIndex &_index) const override;

        void setColumnWidth(KLib::LedgerWidget* _view) const override;
};

#endif // REWARDSLEDGERCONTROLLER_H
