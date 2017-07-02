#ifndef SPREADSHEETMODEL_H
#define SPREADSHEETMODEL_H

#include <QAbstractTableModel>
#include <KangarooLib/amount.h>
#include <QHash>
#include <QColor>
#include <QFont>
#include <QDate>
#include "function.h"

namespace ValueType
{
    enum Type
    {
        Any      = 0,

        Integer  = 1,
        Decimal  = 2,
        Double   = 3,

        String   = 10,
        Date     = 20,        
        Object   = 30,

        None     = 100
    };
}

class Value
{
    public:
        Value() :
            m_type(ValueType::Any),
            m_isFunc(false) {}


        QVariant currentValue() const;

        void setValue(const QVariant _value);

        void setType(const int _type);

        int type() const { return m_type; }

    private:
        int m_type;

        QVariant     val_any;
        int          val_int;
        KLib::Amount val_dec;
        double       val_double;
        QString      val_string;
        QDate        val_date;

        Function     m_func;
        bool         m_isFunc;
};

class Cell
{
    public:
        Cell();

        QVariant dataForRole(int _role) const;
        void setDataForRole(const QVariant& _data, int _role);

        static const QColor DEFAULT_BACKGROUND;
        static const QColor DEFAULT_FOREGROUND;
        static const Qt::Alignment DEFAULT_ALIGN;

    private:
        Value    m_value;

        QColor   m_background;
        QColor   m_foreground;
        QFont    m_font;
        int      m_align;
};

class SpreadsheetModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        explicit SpreadsheetModel(QObject *parent = 0);

        ~SpreadsheetModel();

        int rowCount(const QModelIndex& _parent = QModelIndex()) const;
        int columnCount(const QModelIndex& _parent = QModelIndex()) const;
        QVariant data(const QModelIndex& _index, int _role = Qt::DisplayRole) const;
        QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const;

        Qt::ItemFlags flags(const QModelIndex& _index) const;

        bool setData(const QModelIndex& _index, const QVariant& _value, int _role = Qt::EditRole);

        static const int NUM_ROWS;
        static const int NUM_COLS;

        static const int DEFAULT_COLUMN_WIDTH;
        static const int DEFAULT_ROW_HEIGHT;

        static QString columnIndexToLetterCode(int _index);

    signals:

    public slots:

    private:
        QHash<int, int> m_columnIds;    ///< First is index of column, second is col-id
        QHash<int, int> m_rowIds;       ///< First is index of column, second is row-id
        QHash<QPair<int, int>, Cell*> m_cells; ///< The pair is COL-ROW

        int m_nextColId;
        int m_nextRowId;

        friend class Function;

};

#endif // SPREADSHEETMODEL_H
