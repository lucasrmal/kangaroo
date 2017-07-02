#include "spreadsheetmodel.h"

#include <climits>
#include <QSize>

using namespace KLib;

const int SpreadsheetModel::NUM_ROWS = 32768;
const int SpreadsheetModel::NUM_COLS = 16384;
const int SpreadsheetModel::DEFAULT_COLUMN_WIDTH = 90;
const int SpreadsheetModel::DEFAULT_ROW_HEIGHT = 22;

const QColor Cell::DEFAULT_BACKGROUND = Qt::white;
const QColor Cell::DEFAULT_FOREGROUND = Qt::black;
const Qt::Alignment Cell::DEFAULT_ALIGN = Qt::AlignLeft | Qt::AlignVCenter;

QVariant Value::currentValue() const
{
    if (m_isFunc)
    {
        return m_func.value();
    }
    else
    {
        switch (m_type)
        {
        case ValueType::Any:
            return val_any;

        case ValueType::Integer:
            return val_int;

        case ValueType::Decimal:
            return val_dec.toString();

        case ValueType::Double:
            return val_double;

        case ValueType::String:
            return val_string;

        case ValueType::Date:
            return val_date.toString();

        default:
            return QVariant();
        }
    }
}

void Value::setValue(const QVariant _value)
{
    if (Function::isFunction(_value.toString()))
    {
        m_isFunc = true;
        m_func = Function::evaluate(_value.toString());
    }
    else
    {
        switch (m_type)
        {
        case ValueType::Any:
            val_any = _value;

        case ValueType::Integer:
            val_int = _value.toInt();

        case ValueType::Decimal:
            val_dec = Amount::fromQVariant(_value);

        case ValueType::Double:
            val_double = _value.toDouble();

        case ValueType::String:
            val_string = _value.toString();

        case ValueType::Date:
            val_date = _value.toDate();

        default:
            break;
        }
    }


}

void Value::setType(const int _type)
{
    if (_type == m_type)
        return;

    QVariant current = currentValue();

    m_type = _type;
    setValue(current);
}

Cell::Cell() :
    m_background(DEFAULT_BACKGROUND),
    m_foreground(DEFAULT_FOREGROUND),
    m_align(DEFAULT_ALIGN)
{

}

QVariant Cell::dataForRole(int _role) const
{
    switch (_role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return m_value.currentValue();

    case Qt::UserRole:
        return m_value.type();

    case Qt::FontRole:
        return m_font;

    case Qt::BackgroundRole:
        return m_background;

    case Qt::ForegroundRole:
        return m_foreground;

    case Qt::TextAlignmentRole:
        return (int) m_align;

    default:
        return QVariant();
    }
}

void Cell::setDataForRole(const QVariant& _data, int _role)
{
    switch (_role)
    {
    case Qt::EditRole:
        m_value.setValue(_data);
        break;

    case Qt::UserRole:
        m_value.setType(_data.toInt());
        break;

    case Qt::FontRole:
        m_font = _data.value<QFont>();
        break;

    case Qt::BackgroundRole:
        m_background = _data.value<QColor>();
        break;

    case Qt::ForegroundRole:
        m_foreground = _data.value<QColor>();
        break;

    case Qt::TextAlignmentRole:
        m_align = _data.toInt();
        break;

    default:
        break;

    }
}

SpreadsheetModel::SpreadsheetModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_nextColId(0),
    m_nextRowId(0)
{
}

SpreadsheetModel::~SpreadsheetModel()
{
    for (Cell* c : m_cells.values())
    {
        delete c;
    }
}

int SpreadsheetModel::rowCount(const QModelIndex& _parent) const
{
    Q_UNUSED(_parent)

    return NUM_ROWS;
}

int SpreadsheetModel::columnCount(const QModelIndex& _parent) const
{
    Q_UNUSED(_parent)

    return NUM_COLS;
}

QVariant SpreadsheetModel::data(const QModelIndex &_index, int _role) const
{
    if (!_index.isValid())
        return QVariant();

    if (m_rowIds.contains(_index.row()) && m_columnIds.contains(_index.column())
        && m_cells.contains(qMakePair(m_columnIds[_index.column()], m_rowIds[_index.row()])))
    {
        return m_cells[qMakePair(m_columnIds[_index.column()], m_rowIds[_index.row()])]->dataForRole(_role);
    }
    else
    {
        switch (_role)
        {
        case Qt::BackgroundColorRole:
            return Cell::DEFAULT_BACKGROUND;

        case Qt::ForegroundRole:
            return Cell::DEFAULT_FOREGROUND;

        case Qt::TextAlignmentRole:
            return (int) Cell::DEFAULT_ALIGN;

        case Qt::UserRole:
            return (int) ValueType::None;

        default:
            return QVariant();
        }
    }

}

bool SpreadsheetModel::setData(const QModelIndex& _index, const QVariant& _value, int _role)
{
    if (!_index.isValid())
        return false;

    if (!m_columnIds.contains(_index.column()))
    {
        m_columnIds[_index.column()] = m_nextColId;
        ++m_nextColId;
    }

    if (!m_rowIds.contains(_index.row()))
    {
        m_rowIds[_index.row()] = m_nextRowId;
        ++m_nextRowId;
    }

    bool added = true;

    if (!m_cells.contains(qMakePair(m_columnIds[_index.column()], m_rowIds[_index.row()])))
    {
        if (_value != QVariant())
        {
            m_cells[qMakePair(m_columnIds[_index.column()], m_rowIds[_index.row()])] = new Cell();
        }
        else
        {
            added = false;
        }
    }

    if (added)
    {
        m_cells[qMakePair(m_columnIds[_index.column()], m_rowIds[_index.row()])]->setDataForRole(_value, _role);
        emit dataChanged(_index, _index);
    }

    return true;
}

QVariant SpreadsheetModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    if (_role == Qt::TextAlignmentRole)
    {
        return (int) Qt::AlignHCenter | Qt::AlignVCenter;
    }
    else if (_role != Qt::DisplayRole)
        return QVariant();

    if (_orientation == Qt::Vertical)
    {
        return QString::number(_section+1);
    }
    else
    {
        return columnIndexToLetterCode(_section);
    }
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex& _index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(_index);

    return _index.isValid() ? flags | Qt::ItemIsEditable : flags;
}

QString SpreadsheetModel::columnIndexToLetterCode(int _index)
{
    int column = _index;
    QString text;
    QChar letter;

    letter = 'A' + column%26;
    text.insert(0, letter);

    if (column>=26)
    {
      column = column/26;
      do
      {
         column--;
         QChar letter('A'+column%26);
         text.insert(0, letter);
         column = column/26;
      } while(column>0);
    }

    return text;
}
