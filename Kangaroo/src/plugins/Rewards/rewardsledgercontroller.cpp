#include "rewardsledgercontroller.h"
#include "rewardsprogram.h"
#include "rewardselector.h"
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/ui/widgets/ledgerwidget.h>
#include <QTableView>
#include <QMessageBox>

using namespace KLib;

RewardsLedgerController::RewardsLedgerController(Ledger* _ledger, QObject *_parent) :
    GenericLedgerController(_ledger, new RewardsLedgerBuffer(), _parent),
    m_program(nullptr)
{
    //Get the reward currency
    resetRewardsProgramInfo();

    connect(Account::getTopLevel(), &Account::accountModified, this, &RewardsLedgerController::onAccountModified);
    connect(Account::getTopLevel(), &Account::accountRemoved,  this, &RewardsLedgerController::onAccountRemoved);

    connect(RewardsProgramManager::instance(), &RewardsProgramManager::programRemoved,
            this, &RewardsLedgerController::onRewardsProgramRemoved);
}

void RewardsLedgerController::onAccountModified(Account* _a)
{
    if (_a == account())
    {
        //Check if the rewards program info were modified
        if (account()->properties()->get(RewardsProgram::PROP_REWARDS_SOURCE_ACCOUNT, -1).toInt() != m_idRewardsFrom
            || account()->properties()->get(RewardsProgram::PROP_REWARDS_TARGET_ACCOUNT, -1).toInt() != m_idRewardsFrom
            || (m_program && account()->properties()->get(RewardsProgram::PROP_REWARDS_PROGRAM, -1).toInt() != m_program->id())
            || (!m_program && account()->properties()->get(RewardsProgram::PROP_REWARDS_PROGRAM, -1).toInt() != -1))
        {
            resetRewardsProgramInfo();
        }
    }
}

void RewardsLedgerController::onAccountRemoved(Account* _a)
{
    if (_a->id() == m_idRewardsFrom || _a->id() == m_idRewardsTo)
    {
        resetRewardsProgramInfo();
    }
}

void RewardsLedgerController::onRewardsProgramRemoved(RewardsProgram* _p)
{
    if (_p == m_program)
    {
        resetRewardsProgramInfo();
    }
}

void RewardsLedgerController::resetRewardsProgramInfo()
{
    try
    {
        m_idRewardsFrom = account()->properties() ->get(RewardsProgram::PROP_REWARDS_SOURCE_ACCOUNT, -1).toInt();
        m_idRewardsTo   = account()->properties() ->get(RewardsProgram::PROP_REWARDS_TARGET_ACCOUNT, -1).toInt();

        //Check the accounts are good
        Account::getTopLevel()->account(m_idRewardsFrom);
        Account::getTopLevel()->account(m_idRewardsTo);

        m_rewardCurrency = Account::getTopLevel()->account(m_idRewardsFrom)->mainCurrency();

        m_program = RewardsProgramManager::instance()->get(account()->properties()
                                                           ->get(RewardsProgram::PROP_REWARDS_PROGRAM, -1).toInt());
    }
    catch (ModelException)
    {
        account()->properties()->remove(RewardsProgram::PROP_REWARDS_PROGRAM);
        account()->properties()->remove(RewardsProgram::PROP_REWARDS_SOURCE_ACCOUNT);
        account()->properties()->remove(RewardsProgram::PROP_REWARDS_TARGET_ACCOUNT);

        if (m_program) //If the previous program was good, reset the model, otherwise there is no need to.
        {
            beginResetModel();

            m_idRewardsFrom = Constants::NO_ID;
            m_idRewardsTo   = Constants::NO_ID;
            m_program       = nullptr;
            m_rewardCurrency.clear();

            endResetModel();
        }
        else
        {
            m_idRewardsFrom = Constants::NO_ID;
            m_idRewardsTo   = Constants::NO_ID;
            m_program       = nullptr;
            m_rewardCurrency.clear();
        }
    }
}

int RewardsLedgerController::columnCount(const QModelIndex& _parent) const
{
    Q_UNUSED(_parent)
    return RewardsLedgerColumn::NumColumns;
}

LedgerWidgetDelegate* RewardsLedgerController::buildDelegate(LedgerWidget* _widget) const
{
    return new RewardsLedgerWidgetDelegate(this, _widget);
}

QVariant RewardsLedgerController::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    if (_section == RewardsLedgerColumn::REWARD
        && _orientation == Qt::Horizontal
        && _role == Qt::DisplayRole)
    {
        return tr("Reward");
    }
    else
    {
        return GenericLedgerController::headerData(_section, _orientation, _role);
    }

}

QList<Transaction::Split> RewardsLedgerController::displayedSplits(const Transaction* _tr) const
{
    QList<Transaction::Split> splits = _tr->splits();
    RewardsLedgerBuffer::removeRewardSplits(splits, this);

    //Place the relevant split first.
    int i = 0;

    while (i < splits.count())
    {
        if (splits[i].idAccount == account()->id())
        {
            splits.swap(i, 0);
        }
        else if (_tr->isCurrencyExchange()
                 && Account::accountIsCurrencyTrading(splits[i].idAccount))
        {
            splits.removeAt(i);
            continue;
        }

        ++i;
    }

    return splits;
}

QVariant RewardsLedgerController::cacheData(int _column, int _cacheRow, int _row, bool _editRole) const
{
    if (_column == RewardsLedgerColumn::REWARD && _row == 0)
    {
        if (_cacheRow != newTransactionRow()
            && rewardsProgram()) //Not the new unused row...
        {
            auto splits = m_cache[_cacheRow].transaction()->splits();
            Amount rewardAmount = RewardsLedgerBuffer::removeRewardSplits(splits, this);
            int idTier = RewardsLedgerBuffer::tierIdForAmount(rewardAmount,
                                                   Transaction::totalForAccount(account()->id(),
                                                                                splits),
                                                   this);

            if (_editRole)
            {
                QList<QVariant> list;
                list << idTier;

                if (idTier == Constants::NO_ID)
                    list << rewardAmount.toQVariant();

                return list;
            }
            else
            {
                return rewardAmount != 0 ? rewardAmount.toString()
                                         : "";
            }
        }
        else //New unset row...
        {
            return QVariant();
        }
    }
    else
    {
        return GenericLedgerController::cacheData(_column, _cacheRow, _row, _editRole);
    }
}

Amount RewardsLedgerController::totalChargedForReward(const QModelIndex& _index) const
{
    if (m_buffer->row != Constants::NO_ID && _index.row() == m_buffer->row) //Buffer
    {
        if (m_buffer->splits.count())
        {
            return -Transaction::totalForAccount(account()->id(), m_buffer->splits);
        }
        else
        {
            return m_buffer->debit != 0 ? -m_buffer->debit : m_buffer->credit;
        }
    }
    else if (_index.row() < rowCount()-1) //Cache
    {
        return -Transaction::totalForAccount(account()->id(), m_cache[_index.row()].transaction()->splits());
    }
    else //New line not in edit yet...
    {
        return 0;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////


RewardsLedgerWidgetDelegate::RewardsLedgerWidgetDelegate(const KLib::LedgerController* _controller,
                                                         KLib::LedgerWidget* _widget) :
    LedgerWidgetDelegate(_controller, _widget)
{
}

QWidget* RewardsLedgerWidgetDelegate::createEditor(QWidget* _parent,
                                                   const QStyleOptionViewItem& _option,
                                                   const QModelIndex& _index) const
{
    if (_index.column() == RewardsLedgerColumn::REWARD)
    {
        QList<RewardTier> tiers;
        const RewardsLedgerController* rewCon = static_cast<const RewardsLedgerController*>(_index.model());

        if (rewCon->rewardsProgram())
        {
            tiers = rewCon->rewardsProgram()->tiers();
        }

        RewardSelector* sel = new RewardSelector(tiers, rewCon->rewardCurrency(), _parent);
        sel->setBaseAmount(rewCon->totalChargedForReward(_index));
        setCurrentEditor(sel);
        return sel;
    }
    else
    {
        return LedgerWidgetDelegate::createEditor(_parent, _option, _index);
    }
}

void RewardsLedgerWidgetDelegate::setEditorData(QWidget* _editor, const QModelIndex& _index) const
{
    if (_index.column() == RewardsLedgerColumn::REWARD)
    {
        RewardSelector* sel = qobject_cast<RewardSelector*>(_editor);
        sel->fromVariantList(_index.data(Qt::EditRole).toList());
    }
    else
    {
        LedgerWidgetDelegate::setEditorData(_editor, _index);
    }
}

void RewardsLedgerWidgetDelegate::setModelData(QWidget* _editor, QAbstractItemModel* _model, const QModelIndex &_index) const
{
    if (_index.column() == RewardsLedgerColumn::REWARD)
    {
        RewardSelector* sel = qobject_cast<RewardSelector*>(_editor);
        _model->setData(_index, sel->toVariantList());
    }
    else
    {
        LedgerWidgetDelegate::setModelData(_editor, _model, _index);
    }
}

void RewardsLedgerWidgetDelegate::setColumnWidth(LedgerWidget* _view) const
{
    LedgerWidgetDelegate::setColumnWidth(_view);
    _view->setColumnWidth(RewardsLedgerColumn::REWARD, 90);
}

////////////////////////////////////////////////////////////////////////////

KLib::Amount RewardsLedgerBuffer::computeRewards(const RewardsLedgerController* _controller) const
{
    if (_controller->rewardsProgram())
    {
        if (idTier != Constants::NO_ID)
        {
            try
            {
                RewardTier tier = _controller->rewardsProgram()->tier(idTier);

                Amount total;

                if (splits.count())
                {
                    total = Transaction::totalForAccount(_controller->account()->id(), splits);
                }
                else
                {
                    total = debit != 0 ? debit : -credit;
                }

                return Amount((-1 * total * tier.rate) / 100)
                        .toPrecision(CurrencyManager::instance()->get(_controller->rewardCurrency())->precision());
            }
            catch (...)
            {
                return rewardAmount;
            }
        }
        else
        {
            return rewardAmount;
        }
    }
    else
    {
        return 0;
    }
}

void RewardsLedgerBuffer::clear()
{
    LedgerBuffer::clear();
    idTier = Constants::NO_ID;
    rewardAmount = 0;
}

bool RewardsLedgerBuffer::setData(int _column, int _row, const QVariant& _value, KLib::LedgerController* _controller)
{
    if (_column == RewardsLedgerColumn::REWARD)
    {
        RewardsLedgerController* rewCon = qobject_cast<RewardsLedgerController*>(_controller);

        if (_row == 0 && rewCon->rewardsProgram())
        {
            try
            {
                QList<QVariant> values = _value.toList();

                if (values.length() == 0)
                {
                    return false;
                }

                bool ok;
                if (values[0].toInt(&ok) != Constants::NO_ID && ok) //Selected a tier
                {
                    //Check that the tier exists...
                    rewCon->rewardsProgram()->tier(values[0].toInt());

                    idTier = values[0].toInt();
                    rewardAmount = 0;
                }
                else if (ok
                         && values[0].toInt() == Constants::NO_ID
                         && values.count() == 2) //Custom reward
                {
                    idTier = Constants::NO_ID;

                    rewardAmount = Amount::fromQVariant(values[1])
                            .toPrecision(CurrencyManager::instance()->get(rewCon->rewardCurrency())->precision());
                }

                return true;
            }
            catch (...)
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else if (LedgerBuffer::setData(_column, _row, _value, _controller))
    {
        if (_column == _controller->col_debit() || _column == _controller->col_credit())
        {
            //Update the reward...
            emit _controller->dataChanged(_controller->index(row, RewardsLedgerColumn::REWARD),
                                          _controller->index(row, RewardsLedgerColumn::REWARD));
        }

        return true;
    }
    else
    {
        return false;
    }
}

QVariant RewardsLedgerBuffer::data(int _column, int _row, bool _editRole, const KLib::LedgerController* _controller) const
{
    if (_column == RewardsLedgerColumn::REWARD && _row == 0)
    {
        const RewardsLedgerController* rewCon = static_cast<const RewardsLedgerController*>(_controller);

        if (rewCon->rewardsProgram())
        {
            if (_editRole)
            {
                QList<QVariant> list;
                list << idTier;

                if (idTier == Constants::NO_ID)
                    list << rewardAmount.toQVariant();

                return list;
            }
            else
            {
                return computeRewards(static_cast<const RewardsLedgerController*>(_controller)).toString();
            }
        }
        else
        {
            return QVariant();
        }
    }
    else
    {
        return LedgerBuffer::data(_column, _row, _editRole, _controller);
    }
}

Amount RewardsLedgerBuffer::removeRewardSplits(QList<KLib::Transaction::Split>& _splits,
                                             const RewardsLedgerController* _controller)
{
    if (_controller->rewardsProgram())
    {
        //Find a to and from split, check if their amounts match, and remove them.
        int idTo = -1, idFrom = -1;

        for (int i = 0; i < _splits.count(); ++i)
        {
            Transaction::Split& s = _splits[i];

            if (s.idAccount == _controller->idRewardsFromAccount())
            {
                idFrom = i;
            }
            else if (s.idAccount == _controller->idRewardsToAccount())
            {
                idTo = i;
            }
        }

        //Check if we found them and if the amount matches
        if (idFrom != -1 && idTo != -1
            && _splits[idFrom].amount + _splits[idTo].amount == 0
            && _splits[idFrom].currency == _splits[idTo].currency)
        {
            Amount a = _splits[idTo].amount;

            //We just want idFrom < idTo for removal purposes.
            if (idTo < idFrom)
            {
                std::swap(idFrom, idTo);
            }

            _splits.removeAt(idTo);
            _splits.removeAt(idFrom);

            return a;
        }
    }

    return 0;


//        Amount amount = Amount::fromQVariant(_transaction->properties()->get(RewardsProgram::PROP_TRANS_REWARD_AMOUNT));

//        //Remove the reward splits
//        if (_splits.count())
//        {
//            //Find the reward "TO" account
//            QString currency = "";

//            for (int i = 0; i < _splits.count(); ++i)
//            {
//                Transaction::Split& s = _splits[i];
//                Account* a = Account::getTopLevel()->account(s.idAccount);

//                if (a->type() == RewardsProgram::REWARDS_ACCOUNT_TYPE
//                    && s.amount == amount)
//                {
//                    currency = a->mainCurrency();
//                    _splits.removeAt(i);
//                    break;
//                }
//            }

//            //Find the reward "FROM" account
//            if (!currency.isEmpty())
//            {
//                for (int i = 0; i < _splits.count(); ++i)
//                {
//                    Transaction::Split& s = _splits[i];
//                    Account* a = Account::getTopLevel()->account(s.idAccount);

//                    if (a->type() == AccountType::INCOME
//                        && a->mainCurrency() == currency
//                        && s.amount == -amount)
//                    {
//                        _splits.removeAt(i);
//                        break;
//                    }
//                }
//            }
//        }
}

int RewardsLedgerBuffer::tierIdForAmount(const KLib::Amount& _rewardAmount,
                                         const KLib::Amount& _transactionAmount,
                                         const RewardsLedgerController* _controller)
{
    int precRew = CurrencyManager::instance()->get(_controller->rewardCurrency())->precision();

    for (const RewardTier& t : _controller->rewardsProgram()->tiers())
    {
        if (Amount((-1 * t.rate * _transactionAmount)/100.0).toPrecision(precRew) == _rewardAmount)
        {
            return t.id;
        }
    }

    return Constants::NO_ID;
}

void RewardsLedgerBuffer::loadSplits(const QList<Transaction::Split>& _splits, const KLib::LedgerController* _controller)
{
    const RewardsLedgerController* rewCon = static_cast<const RewardsLedgerController*>(_controller);
    QList<Transaction::Split> tempSplits = _splits;

    //Load the reward part
    if (rewCon->rewardsProgram() && tempSplits.count() > 2) //If <= 2 splits, then there were no reward splits included
    {
        //Remove the reward splits if they do exist
        rewardAmount = removeRewardSplits(tempSplits, rewCon);
        idTier = tierIdForAmount(rewardAmount,
                                 Transaction::totalForAccount(_controller->account()->id(),
                                                              tempSplits),
                                 rewCon);

    }
    else
    {
        idTier = Constants::NO_ID;
        rewardAmount = 0;
    }

    //Now we can just load as usual.
    LedgerBuffer::loadSplits(tempSplits, _controller);
}

QStringList RewardsLedgerBuffer::validate(int& _firstErrorColumn, const LedgerController* _controller)
{
    QStringList errors = LedgerBuffer::validate(_firstErrorColumn, _controller);
    const RewardsLedgerController* rewCon = static_cast<const RewardsLedgerController*>(_controller);

    //Check the reward tier
    if (rewCon->rewardsProgram())
    {
        if (idTier != Constants::NO_ID)
        {
            try
            {
                //Try to get the tier and rewards program
                rewCon->rewardsProgram()->tier(idTier);
            }
            catch (ModelException)
            {
                errors << QObject::tr("The reward tier selected is invalid.");

                if (_firstErrorColumn == -1)
                {
                    _firstErrorColumn = RewardsLedgerColumn::REWARD;
                }
            }
        }
    }

    return errors;
}

QList<Transaction::Split> RewardsLedgerBuffer::splitsForSaving(LedgerController* _controller, bool& _ok) const
{
    QList<Transaction::Split> tmpSplits = LedgerBuffer::splitsForSaving(_controller, _ok);

    if (!_ok)
    {
        return QList<Transaction::Split>();
    }

    const RewardsLedgerController* rewCon = static_cast<const RewardsLedgerController*>(_controller);

    //Add the rebate splits if necessary
    if (rewCon->rewardsProgram()
        && (idTier != Constants::NO_ID || rewardAmount != 0))
    {
        Amount a = computeRewards(rewCon);
        tmpSplits << Transaction::Split(-a,
                                        rewCon->idRewardsFromAccount(),
                                        rewCon->rewardCurrency());

        tmpSplits << Transaction::Split(a,
                                        rewCon->idRewardsToAccount(),
                                        rewCon->rewardCurrency());
    }

    _ok = true;
    return tmpSplits;
}

//QHash<QString, QVariant> RewardsLedgerBuffer::propertiesForSaving(LedgerController* _controller) const
//{
//    QHash<QString, QVariant> prop = LedgerBuffer::propertiesForSaving(_controller);
//    const RewardsLedgerController* rewCon = static_cast<const RewardsLedgerController*>(_controller);

//    if (rewCon->rewardsProgram() != Constants::NO_ID
//        && (idTier != Constants::NO_ID || rewardAmount != 0))
//    {
//        prop[RewardsProgram::PROP_REWARDS_PROGRAM] = rewCon->rewardsProgram();
//        prop[RewardsProgram::PROP_TRANS_ID_TIER] = idTier;
//        prop[RewardsProgram::PROP_TRANS_REWARD_AMOUNT] = computeRewards(static_cast<RewardsLedgerController*>(_controller)).toQVariant();
//    }

//    return prop;
//}


