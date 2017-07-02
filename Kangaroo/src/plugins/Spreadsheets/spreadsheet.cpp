#include "spreadsheet.h"
#include "spreadsheetmodel.h"
#include <QHeaderView>
#include <QKeyEvent>

Spreadsheet::Spreadsheet(QWidget *parent) :
    QTableView(parent)
{
    verticalHeader()->setDefaultSectionSize(SpreadsheetModel::DEFAULT_ROW_HEIGHT);
    horizontalHeader()->setDefaultSectionSize(SpreadsheetModel::DEFAULT_COLUMN_WIDTH);
    setModel(new SpreadsheetModel(this));
}

void Spreadsheet::keyPressEvent(QKeyEvent* _event)
{
    switch (_event->key())
    {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        deleteContentsOfSelection();
        break;

    default:
        QTableView::keyPressEvent(_event);
        return;
    }

    _event->accept();
}

bool Spreadsheet::hasSelectedCells() const
{
    return selectionModel()->hasSelection();
}

void Spreadsheet::deleteContentsOfSelection() const
{
    for (QModelIndex i : selectionModel()->selectedIndexes())
    {
        model()->setData(i, QVariant(), Qt::EditRole);
    }
}

void Spreadsheet::setSelectionFont(const QFont& _font)
{
    for (QModelIndex i : selectionModel()->selectedIndexes())
    {
        model()->setData(i, _font, Qt::FontRole);
    }
}

void Spreadsheet::setSelectionFont(bool _value, FontType _type)
{
    for (QModelIndex i : selectionModel()->selectedIndexes())
    {
        QFont f = model()->data(i, Qt::FontRole).value<QFont>();

        switch (_type)
        {
        case Bold:
            f.setBold(_value);
            break;
        case Italic:
            f.setItalic(_value);
            break;
        case Underline:
            f.setUnderline(_value);
            break;
        }

        model()->setData(i, f, Qt::FontRole);
    }
}

void Spreadsheet::setSelectionColor(const QColor& _color, ColorType _type)
{
    for (QModelIndex i : selectionModel()->selectedIndexes())
    {
        model()->setData(i, _color, _type == Background ? Qt::BackgroundRole : Qt::ForegroundRole);
    }
}

void Spreadsheet::setSelectionAlignment(Qt::Alignment _align)
{
    for (QModelIndex i : selectionModel()->selectedIndexes())
    {
        model()->setData(i, (int) _align, Qt::TextAlignmentRole);
    }
}

QFont Spreadsheet::selectionFont() const
{
    bool first = true;
    QFont f;

    for (QModelIndex i : selectionModel()->selectedIndexes())
    {
        //Only look at non null cells
        if (model()->data(i, Qt::UserRole).toInt() != ValueType::None)
        {
            QFont curFont = model()->data(i, Qt::FontRole).value<QFont>();

            //Check if the font is the same as current. If not, return default font.
            if (first)
            {
                f = curFont;
                first = false;
            }
            else if (curFont != f)
            {
                return QFont();
            }
        }
    }

    //This will only be executed if the font is the same for all non null cells.
    return f;
}

bool Spreadsheet::selectionFont(FontType _type) const
{
    bool first = true;
    bool val;

    for (QModelIndex i : selectionModel()->selectedIndexes())
    {
        //Only look at non null cells
        if (model()->data(i, Qt::UserRole).toInt() != ValueType::None)
        {
            QFont curFont = model()->data(i, Qt::FontRole).value<QFont>();

            //Check if the font is the same as current. If not, return default font.
            if (first)
            {
                switch (_type)
                {
                case Bold:
                    val = curFont.bold();
                    break;

                case Italic:
                    val = curFont.italic();
                    break;

                case Underline:
                    val = curFont.underline();
                    break;
                }

                first = false;
            }
            else
            {
                bool cur;
                switch (_type)
                {
                case Bold:
                    cur = curFont.bold();
                    break;

                case Italic:
                    cur = curFont.italic();
                    break;

                case Underline:
                    cur = curFont.underline();
                    break;
                }

                if (cur != val)
                {
                    return false;
                }
            }
        }
    }

    //This will only be executed if the font is the same for all non null cells.
    return val;
}

QColor Spreadsheet::selectionColor(ColorType _type) const
{
    bool first = true;
    QColor val;

    for (QModelIndex i : selectionModel()->selectedIndexes())
    {
        //Only look at non null cells
        if (model()->data(i, Qt::UserRole).toInt() != ValueType::None)
        {
            QColor curColor = model()->data(i, _type == Background ? Qt::BackgroundRole
                                                                   : Qt::ForegroundRole).value<QColor>();

            //Check if the color is the same as current. .
            if (first)
            {
                val = curColor;
                first = false;
            }
            else if (curColor != val)
            {
                //If not, return default color
                return  _type == Background ? Cell::DEFAULT_BACKGROUND
                                            : Cell::DEFAULT_FOREGROUND;
            }
        }
    }

    //This will only be executed if the color is the same for all non null cells.
    return val;
}

