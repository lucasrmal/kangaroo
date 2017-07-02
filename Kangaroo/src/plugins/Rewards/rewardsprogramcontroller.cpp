#include "rewardsprogramcontroller.h"
#include "rewardsprogram.h"

#include <KangarooLib/model/currency.h>
#include <KangarooLib/controller/currencycontroller.h>
#include <QSortFilterProxyModel>
#include <algorithm>
#include <QComboBox>

using namespace KLib;

RewardsProgramController::RewardsProgramController(QObject *parent)
    : QAbstractTableModel(parent),
      m_manager(RewardsProgramManager::instance())
{
    connect(m_manager, &RewardsProgramManager::programAdded, this, &RewardsProgramController::onProgramAdded);
    connect(m_manager, &RewardsProgramManager::programModified, this, &RewardsProgramController::onProgramModified);
    connect(m_manager, &RewardsProgramManager::programRemoved, this, &RewardsProgramController::onProgramRemoved);

    //Load
    m_sorted = m_manager->programs();
    std::sort(m_sorted.begin(), m_sorted.end(), [](RewardsProgram* _a, RewardsProgram* _b)
                                                { return _a->name() < _b->name(); });
}

QSortFilterProxyModel* RewardsProgramController::sortProxy(QObject* _parent)
{
    QSortFilterProxyModel* proxyRewards = new QSortFilterProxyModel(_parent);
    proxyRewards->setSourceModel(new RewardsProgramController(_parent));
    proxyRewards->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyRewards->sort(0);
    return proxyRewards;
}

int RewardsProgramController::rowCount(const QModelIndex&) const
{
    return m_sorted.count();
}

int RewardsProgramController::columnCount(const QModelIndex&) const
{
    return RewardsProgramColumn::NumColumns;
}

QVariant RewardsProgramController::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_sorted.count())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (index.column())
        {
        case RewardsProgramColumn::NAME:
            return m_sorted[index.row()]->name();

        case RewardsProgramColumn::CURRENCY:
            if (role == Qt::DisplayRole)
            {
                try
                {
                    return CurrencyManager::instance()->get(m_sorted[index.row()]->currency())->name();
                }
                catch (...)
                {
                    return QVariant();
                }
            }
            else
            {
                return m_sorted[index.row()]->currency();
            }

        default:
            return QVariant();
        }

    }
    else if (role == Qt::UserRole)
    {
        return m_sorted[index.row()]->id();
    }
    else
    {
        return QVariant();
    }
}

QVariant RewardsProgramController::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case RewardsProgramColumn::NAME:
            return tr("Name");

        case RewardsProgramColumn::CURRENCY:
            return tr("Currency");

        default:
            return QVariant();
        }
    }
    else
    {
        return QString("%1").arg(section+1);
    }
}

Qt::ItemFlags RewardsProgramController::flags(const QModelIndex &index) const
{
    if (!index.isValid() ||
        index.row() >= m_sorted.count() ||
        (index.column() == RewardsProgramColumn::CURRENCY &&
         m_sorted[index.row()]->isInUse()))
    {
        return QAbstractItemModel::flags(index);
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool RewardsProgramController::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        switch (index.column())
        {
        case RewardsProgramColumn::NAME:
            m_sorted[index.row()]->setName(value.toString());
            break;

        case RewardsProgramColumn::CURRENCY:
            try
            {
                m_sorted[index.row()]->setCurrency(value.toString());
                break;
            }
            catch (...) { return false; }

        default:
            return false;
        }

        emit dataChanged(index, index);
        return true;
    }
    return false;
}

void RewardsProgramController::onProgramAdded(RewardsProgram* _p)
{

    //Find where it goes
    auto i = m_sorted.begin();
    int rowNo = 0;
    for (;i != m_sorted.end(); ++i, ++rowNo)
    {
        if (_p->name().compare((*i)->name(), Qt::CaseInsensitive) < 0)
            break;
    }

    //Insert it there...
    beginInsertRows(QModelIndex(), rowNo, rowNo);
    m_sorted.insert(i, _p);

    endInsertRows();
}

void RewardsProgramController::onProgramRemoved(RewardsProgram* _p)
{
    int rowNo = m_sorted.indexOf(_p);
    beginRemoveRows(QModelIndex(), rowNo, rowNo);
    m_sorted.removeAt(rowNo);
    endRemoveRows();
}

void RewardsProgramController::onProgramModified(RewardsProgram* _p)
{
    int rowNo = m_sorted.indexOf(_p);
    emit dataChanged(createIndex(rowNo,0), createIndex(rowNo, 0));
}

/////////////////////////////////////////////////////////////////////////////////////////

RewardsProgramDelegate::RewardsProgramDelegate(QObject* _parent) :
    QItemDelegate(_parent)
{
}

QWidget* RewardsProgramDelegate::createEditor(QWidget* _parent,
                                            const QStyleOptionViewItem& _option,
                                            const QModelIndex& _index) const
{
    switch (_index.column())
    {
    case RewardsProgramColumn::CURRENCY:
    {
        QComboBox* cbo = new QComboBox(_parent);
        cbo->setModel(CurrencyController::sortProxy(cbo));
        cbo->setModelColumn(CurrencyColumn::NAME);
        cbo->setFocus();
        return cbo;
    }
    default:
        return QItemDelegate::createEditor(_parent, _option, _index);
    }
}

void RewardsProgramDelegate::setEditorData(QWidget* _editor, const QModelIndex& _index) const
{
    switch (_index.column())
    {
    case RewardsProgramColumn::CURRENCY:
    {
        QComboBox* cbo =  static_cast<QComboBox*>(_editor);
        cbo->setCurrentIndex(cbo->findData(_index.model()->data(_index, Qt::EditRole)));
        break;
    }
    default:
        return QItemDelegate::setEditorData(_editor, _index);
    }
}
void RewardsProgramDelegate::setModelData(QWidget* _editor,
                                        QAbstractItemModel* _model,
                                        const QModelIndex& _index) const
{
    switch (_index.column())
    {
    case RewardsProgramColumn::CURRENCY:
    {
        QComboBox* cbo =  static_cast<QComboBox*>(_editor);
        _model->setData(_index, cbo->currentData(), Qt::EditRole);
        break;
    }
    default:
        return QItemDelegate::setModelData(_editor,_model, _index);
    }
}

void RewardsProgramDelegate::updateEditorGeometry(QWidget* _editor,
                                                const QStyleOptionViewItem& _option,
                                                const QModelIndex& _index) const
{
    Q_UNUSED(_index)
    _editor->setGeometry(_option.rect);
}
