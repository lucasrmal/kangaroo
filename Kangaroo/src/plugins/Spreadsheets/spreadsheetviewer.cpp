#include "spreadsheetviewer.h"
#include "spreadsheet.h"

#include <KangarooLib/ui/core.h>

#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>
#include <QFontDialog>
#include <QColorDialog>

#define CREATE_ACTION(_name, _icon, _text, _slot)   _name = new QAction(Core::icon(_icon), _text, m_toolbar); \
                                                    connect(_name, SIGNAL(triggered()), this, SLOT(_slot)); \
                                                    m_toolbar->addAction(_name);

using namespace KLib;

SpreadsheetViewer::SpreadsheetViewer(QWidget* _parent) :
    QWidget(_parent)
{
    //Create the layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    m_toolbar = new QToolBar(this);
    m_spreadsheet = new Spreadsheet(this);

    layout->addWidget(m_toolbar);
    layout->addWidget(m_spreadsheet);

    layout->setContentsMargins(0,0,0,0);

    //Create the actions
    CREATE_ACTION(act_alignLeft, "format-justify-left", tr("Align Left"), alignLeft());
    CREATE_ACTION(act_alignCenter, "format-justify-center", tr("Align Center"), alignCenter());
    CREATE_ACTION(act_alignRight, "format-justify-right", tr("Align Right"), alignRight());
    m_toolbar->addSeparator();
    CREATE_ACTION(act_toggleBold, "format-text-bold", tr("Bold"), toggleBold());
    CREATE_ACTION(act_toggleItalic, "format-text-italic", tr("Italic"), toggleItalic());
    CREATE_ACTION(act_toggleUnderline, "format-text-underline", tr("Underline"), toggleUnderline());
    CREATE_ACTION(act_changeFont, "format-text-font", tr("Font"), changeFont());
    m_toolbar->addSeparator();
    CREATE_ACTION(act_changeBackground, "format-fill-color", tr("Background Color"), changeBackground());
    CREATE_ACTION(act_changeForeground, "format-text-color", tr("Text Color"), changeForeground());


}

void SpreadsheetViewer::alignLeft()
{
    m_spreadsheet->setSelectionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

void SpreadsheetViewer::alignCenter()
{
    m_spreadsheet->setSelectionAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

void SpreadsheetViewer::alignRight()
{
    m_spreadsheet->setSelectionAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

void SpreadsheetViewer::toggleBold()
{
    bool isBold = m_spreadsheet->selectionFont(Bold);
    m_spreadsheet->setSelectionFont(!isBold, Bold);
}

void SpreadsheetViewer::toggleItalic()
{
    bool isBold = m_spreadsheet->selectionFont(Italic);
    m_spreadsheet->setSelectionFont(!isBold, Italic);
}

void SpreadsheetViewer::toggleUnderline()
{
    bool isBold = m_spreadsheet->selectionFont(Underline);
    m_spreadsheet->setSelectionFont(!isBold, Underline);
}

void SpreadsheetViewer::changeFont()
{
    QFont current = m_spreadsheet->selectionFont();
    bool ok;

    QFont n = QFontDialog::getFont(&ok,
                                   current,
                                   this,
                                   tr("Select Font"));

    if (ok)
    {
        m_spreadsheet->setSelectionFont(n);
    }
}

void SpreadsheetViewer::changeBackground()
{
    QColor current = m_spreadsheet->selectionColor(Background);

    QColor n = QColorDialog::getColor(current,
                                      this,
                                      tr("Select Background Color"));

    if (n.isValid())
    {
        m_spreadsheet->setSelectionColor(n, Background);
    }
}

void SpreadsheetViewer::changeForeground()
{
    QColor current = m_spreadsheet->selectionColor(Foreground);

    QColor n = QColorDialog::getColor(current,
                                      this,
                                      tr("Select Text Color"));

    if (n.isValid())
    {
        m_spreadsheet->setSelectionColor(n, Foreground);
    }
}

