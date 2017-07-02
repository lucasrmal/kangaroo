#include "tiereditor.h"
#include <KangarooLib/ui/widgets/amountedit.h>
#include <KangarooLib/ui/core.h>
#include <QMessageBox>
#include <QHeaderView>
#include <QPushButton>

using namespace KLib;

TierController::TierController(QList<RewardTier>& _tiers, QObject* _parent)
    : QAbstractTableModel(_parent),
      m_tiers(_tiers)
{
    ensureOneEmptyRow();
}

void TierController::reload()
{
    beginResetModel();
    ensureOneEmptyRow();
    endResetModel();
}

int TierController::rowCount(const QModelIndex& _parent) const
{
    Q_UNUSED(_parent)
    return m_tiers.count();
}

int TierController::columnCount(const QModelIndex& _parent) const
{
    Q_UNUSED(_parent)
    return TierColumn::NumColumns;
}

QVariant TierController::data(const QModelIndex& _index, int _role) const
{
    if (_role != Qt::EditRole && _role != Qt::DisplayRole)
        return QVariant();

    if (_index.isValid() && _index.row() < rowCount())
    {
        switch (_index.column())
        {
        case TierColumn::NAME:
            return m_tiers[_index.row()].name;

        case TierColumn::RATE:
            if (m_tiers[_index.row()].rate == 0 && _role == Qt::DisplayRole)
            {
                return QVariant();
            }
            else
            {
                return m_tiers[_index.row()].rate.toString();
            }

        default:
            return QVariant();
        }
    }

    return QVariant();
}

QVariant TierController::headerData(int _section,
                                      Qt::Orientation _orientation,
                                      int _role) const
{
    if (_role != Qt::DisplayRole)
        return QVariant();

    if (_orientation == Qt::Horizontal)
    {
        switch (_section)
        {
        case TierColumn::NAME:
            return tr("Name");

        case TierColumn::RATE:
            return tr("Rate (%)");

        default:
            return QVariant();
        }
    }
    else
    {
        return QString("%1").arg(_section+1);
    }
}

Qt::ItemFlags TierController::flags(const QModelIndex& _index) const
{
    if (_index.isValid())
    {
        return QAbstractTableModel::flags(_index) | Qt::ItemIsEditable;
    }
    else
    {
        return QAbstractTableModel::flags(_index);
    }
}

bool TierController::setData(const QModelIndex& _index,
                               const QVariant& _value,
                               int _role)
{
    if (_index.isValid() && _index.row() < rowCount() && _role == Qt::EditRole)
    {
        switch (_index.column())
        {
        case TierColumn::NAME:
            m_tiers[_index.row()].name = _value.toString();
            break;

        case TierColumn::RATE:
            m_tiers[_index.row()].rate = Amount::fromUserLocale(_value.toString());
            break;

        default:
            return false;
        }
    }

    emit dataChanged(_index, _index);

    //If editing last row and is not empty, we add one
    if (_index.row() == rowCount()-1 && !rowIsEmpty(_index.row()))
    {
        insertRows(rowCount(), 1);
    }
    else if (rowIsEmpty(rowCount()-1) && rowIsEmpty(rowCount()-2))
    {
        //If the two last rows are empty, remove one.
        removeRows(rowCount()-1, 1);
    }
    return true;
}

bool TierController::rowIsEmpty(int _row)
{
    if (_row < 0 || _row >= rowCount())
        return false;

    return m_tiers[_row].rate == 0
           && m_tiers[_row].name.isEmpty();
}

void TierController::ensureOneEmptyRow()
{
    if (m_tiers.count() == 0 || !rowIsEmpty(rowCount()-1))
    {
        insertRows(rowCount(), 1);
    }
}

bool TierController::insertRows(int _row, int _count, const QModelIndex& _parent)
{
    beginInsertRows(_parent, _row, _row+_count-1);
    for (int i = 0; i < _count; ++i)
    {
        m_tiers.insert(_row, RewardTier());
    }
    endInsertRows();

    return true;
}

bool TierController::removeRows(int _row, int _count, const QModelIndex& _parent)
{
    beginRemoveRows(_parent, _row, _row+_count-1);
    for (int i = 0; i < _count; ++i)
    {
        m_tiers.removeAt(_row);
    }
    endRemoveRows();

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

TierEditorDelegate::TierEditorDelegate(QObject* _parent) :
    QItemDelegate(_parent)
{
}

QWidget* TierEditorDelegate::createEditor(QWidget* _parent,
                                            const QStyleOptionViewItem& _option,
                                            const QModelIndex& _index) const
{
    switch (_index.column())
    {
    case TierColumn::RATE:
    {
        AmountEdit* amtEdit = new AmountEdit(2, _parent);
        amtEdit->setMinimum(-10000);
        amtEdit->setFocus();
        return amtEdit;
    }
    default:
        return QItemDelegate::createEditor(_parent, _option, _index);
    }
}

void TierEditorDelegate::setEditorData(QWidget* _editor, const QModelIndex& _index) const
{
    switch (_index.column())
    {
    case TierColumn::RATE:
    {
        Amount value = Amount::fromUserLocale(_index.model()->data(_index, Qt::EditRole).toString());
        AmountEdit* amtEdit =  static_cast<AmountEdit*>(_editor);
        amtEdit->setAmount(value);
        break;
    }
    default:
        return QItemDelegate::setEditorData(_editor, _index);
    }
}
void TierEditorDelegate::setModelData(QWidget* _editor,
                                        QAbstractItemModel* _model,
                                        const QModelIndex& _index) const
{
    switch (_index.column())
    {
    case TierColumn::RATE:
    {
        AmountEdit* amtEdit =  static_cast<AmountEdit*>(_editor);
        _model->setData(_index, amtEdit->amount().toString(), Qt::EditRole);
        break;
    }
    default:
        return QItemDelegate::setModelData(_editor,_model, _index);
    }
}

void TierEditorDelegate::updateEditorGeometry(QWidget* _editor,
                                                const QStyleOptionViewItem& _option,
                                                const QModelIndex& _index) const
{
    Q_UNUSED(_index)
    _editor->setGeometry(_option.rect);
}

/////////////////////////////////////////////////////////////////////////////////////////

TierEditor::TierEditor(QList<RewardTier>& _tiers, QWidget *parent) :
    QTableView(parent),
    m_tiers(_tiers),
    m_model(new TierController(_tiers, this))
{
    setModel(m_model);
    setItemDelegate(&m_delegate);

    setColumnWidth(TierColumn::NAME, 200);
    setColumnWidth(TierColumn::RATE, 100);
    horizontalHeader()->setSectionResizeMode(TierColumn::NAME, QHeaderView::Stretch);

    setSelectionBehavior(QAbstractItemView::SelectRows);
}

bool TierEditor::validate()
{
    QList<RewardTier> notEmpty;
    QString msgBoxTitle = tr("Save Tiers");

    for (RewardTier& t: m_tiers)
    {
        if (!t.name.isEmpty() || t.rate != 0)
        {
            if (t.name.isEmpty())
            {
                QMessageBox::information(this, msgBoxTitle, tr("Enter a name for this tier."));
                edit(m_model->index(currentIndex().row(), TierColumn::NAME));
                return false;
            }
            else if (t.rate == 0)
            {
                QMessageBox::information(this, msgBoxTitle, tr("Enter a non-zero rate for this tier"));
                edit(m_model->index(currentIndex().row(), TierColumn::RATE));
                return false;
            }

            notEmpty << t;
        }
    }

    m_tiers.clear();
    m_tiers.append(notEmpty);
    return true;
}

void TierEditor::addRow()
{
    m_model->insertRow(m_model->rowCount());
}

void TierEditor::removeCurrentRow()
{
    m_model->removeRow(currentIndex().row());
}

void TierEditor::reload()
{
    m_model->reload();
}

/////////////////////////////////////////////////////////////////////////////////////////

FormTierEditor::FormTierEditor(QList<RewardTier>& _tiers, QWidget* _parent) :
    CAMSEGDialog(DialogWithPicture,
                 OkCancelButtons,
                 _parent)
{
    setBothTitles(tr("Edit Tiers"));
    setPicture(Core::pixmap("tiers"));

    m_tierEditor = new TierEditor(_tiers, this);

    setCentralWidget(m_tierEditor);

//    if (!_readOnly)
//    {
        m_btnInsert = new QPushButton(Core::icon("list-add"), "", this);
        m_btnRemove = new QPushButton(Core::icon("list-remove"), "", this);

        connect(m_btnInsert, SIGNAL(clicked()), m_tierEditor, SLOT(addRow()));
        connect(m_btnRemove, SIGNAL(clicked()), m_tierEditor, SLOT(removeCurrentRow()));

        addButton(m_btnInsert, 0, CAMSEGDialog::AtLeft);
        addButton(m_btnRemove, 1, CAMSEGDialog::AtLeft);
//    }
}

void FormTierEditor::accept()
{
    if (m_tierEditor->validate())
    {
        done(QDialog::Accepted);
    }
}

