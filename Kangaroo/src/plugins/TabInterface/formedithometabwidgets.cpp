#include "formedithometabwidgets.h"
#include "hometab.h"

#include <KangarooLib/ui/core.h>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

using namespace KLib;

FormEditHomeTabWidgets::FormEditHomeTabWidgets(HomeTab* _parent) :
    CAMSEGDialog(DialogWithPicture, OkCancelButtons, _parent),
    m_homeTab(_parent)
{
    setTitle(tr("Frequent Accounts"));
    setWindowTitle(title());
    setPicture(Core::pixmap("favorites"));

    m_listAll      = new QListWidget(this);
    m_listSelected = new QListWidget(this);

    m_btnSelect   = new QPushButton(Core::icon("go-right"), "", this);
    m_btnUnselect = new QPushButton(Core::icon("go-left"), "", this);
    m_btnMoveUp   = new QPushButton(Core::icon("go-up"), "", this);
    m_btnMoveDown = new QPushButton(Core::icon("go-down"), "", this);

    connect(m_btnSelect,    &QPushButton::clicked, this, &FormEditHomeTabWidgets::select);
    connect(m_btnUnselect,  &QPushButton::clicked, this, &FormEditHomeTabWidgets::unselect);
    connect(m_btnMoveUp,    &QPushButton::clicked, this, &FormEditHomeTabWidgets::moveUp);
    connect(m_btnMoveDown,  &QPushButton::clicked, this, &FormEditHomeTabWidgets::moveDown);

    m_listAll->setIconSize(QSize(48, 48));
    m_listSelected->setIconSize(QSize(48, 48));

    QVBoxLayout* buttonLayout = new QVBoxLayout();
    buttonLayout->addStretch(5);
    buttonLayout->addWidget(m_btnSelect);
    buttonLayout->addWidget(m_btnUnselect);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(m_btnMoveUp);
    buttonLayout->addWidget(m_btnMoveDown);
    buttonLayout->addStretch(5);

    QLabel* lblAll      = new QLabel(tr("Unused Widgets"), this);
    QLabel* lblSelected = new QLabel(tr("Displayed Widgets"), this);

    QFont bold;
    bold.setBold(true);

    lblAll->setFont(bold);
    lblSelected->setFont(bold);

    QVBoxLayout* layLeft = new QVBoxLayout();
    layLeft->addWidget(lblAll);
    layLeft->addWidget(m_listAll);

    QVBoxLayout* layRight = new QVBoxLayout();
    layRight->addWidget(lblSelected);
    layRight->addWidget(m_listSelected);

    QHBoxLayout* lay = new QHBoxLayout(centralWidget());
    lay->addLayout(layLeft);
    lay->addLayout(buttonLayout);
    lay->addLayout(layRight);
    centralWidget()->setLayout(lay);

    loadWidgets();

    connect(m_listAll,      &QListWidget::currentRowChanged, this, &FormEditHomeTabWidgets::onRowChanged_All);
    connect(m_listSelected, &QListWidget::currentRowChanged, this, &FormEditHomeTabWidgets::onRowChanged_Selected);

    onRowChanged_All(-1);
    onRowChanged_Selected(-1);

}

void FormEditHomeTabWidgets::loadWidgets()
{
    QSet<QString> displayed;
    QFont boldFont;
    boldFont.setBold(true);

    //Load the displayed widgets first
    for (const HomeTab::HomeWidgetPair& p : m_homeTab->m_displayedWidgets)
    {
        displayed << p.first;

        HomeWidgetInfo info = m_homeTab->m_homeWidgets[p.first];

        QListWidgetItem* i = new QListWidgetItem(info.icon,
                                                 info.name,
                                                 m_listSelected);
        i->setData(Qt::UserRole, info.code);
        i->setData(Qt::ToolTipRole, info.description);
        i->setData(Qt::FontRole, boldFont);
    }

    //Now the "all", skipping the displayed.
    for (const HomeWidgetInfo& info : m_homeTab->m_homeWidgets)
    {
        if (!displayed.contains(info.code))
        {
            QListWidgetItem* i = new QListWidgetItem(info.icon,
                                                     info.name,
                                                     m_listAll);
            i->setData(Qt::UserRole, info.code);
            i->setData(Qt::ToolTipRole, info.description);
            i->setData(Qt::FontRole, boldFont);
        }

    }
}

void FormEditHomeTabWidgets::accept()
{
    QList<QString> newWidgets;
    QSet<QString> deletedWidgets;
    QHash<QString, IHomeWidget*> currentWidgets;

    for (int i = 0; i < m_listSelected->count(); ++i)
    {
        newWidgets.append(m_listSelected->item(i)->data(Qt::UserRole).toString());
    }

    for (int i = 0; i < m_listAll->count(); ++i)
    {
        deletedWidgets.insert(m_listAll->item(i)->data(Qt::UserRole).toString());
    }

    //First, remove everything from the layout
    auto i = m_homeTab->m_displayedWidgets.begin();
    while (i != m_homeTab->m_displayedWidgets.end())
    {
        const HomeTab::HomeWidgetPair& p = *i;
        m_homeTab->m_layout->removeWidget(p.second);

        if (deletedWidgets.contains(p.first))
        {
            p.second->deleteLater();
            i = m_homeTab->m_displayedWidgets.erase(i);
        }
        else
        {
            currentWidgets[p.first] = p.second;
            ++i;
        }
    }

    //Add back the new ones
    int idxDis = 0;
    m_homeTab->m_displayedWidgets.clear();

    for (const QString& code : newWidgets)
    {
        IHomeWidget* w = currentWidgets.contains(code) ? currentWidgets[code]
                                                       : m_homeTab->m_homeWidgets[code].build(m_homeTab);
        w->setMaximumHeight(HomeTab::MAX_WIDGET_HEIGHT);

        m_homeTab->m_layout->insertWidget(++idxDis, w);
        m_homeTab->m_displayedWidgets.append(HomeTab::HomeWidgetPair(code, w));
    }

    done(QDialog::Accepted);
}

void FormEditHomeTabWidgets::onRowChanged_All(int _row)
{
    m_btnSelect->setEnabled(_row >= 0);
}

void FormEditHomeTabWidgets::onRowChanged_Selected(int _row)
{
    m_btnMoveDown->setEnabled(_row != -1 && _row < m_listSelected->count()-1);
    m_btnMoveUp->setEnabled(_row > 0);
    m_btnUnselect->setEnabled(_row >= 0);
}

void FormEditHomeTabWidgets::select()
{
    if (m_listAll->currentRow()!= -1)
    {
        m_listSelected->addItem(m_listAll->takeItem(m_listAll->currentRow()));
    }
}

void FormEditHomeTabWidgets::unselect()
{
    if (m_listSelected->currentRow()!= -1)
    {
        m_listAll->addItem(m_listSelected->takeItem(m_listSelected->currentRow()));
        m_listAll->sortItems();
    }

}

void FormEditHomeTabWidgets::moveUp()
{
    int row = m_listSelected->currentRow();

    if (row > 0)
    {
        m_listSelected->insertItem(row-1, m_listSelected->takeItem(row));
        m_listSelected->setCurrentRow(row-1);
    }
}

void FormEditHomeTabWidgets::moveDown()
{
    int row = m_listSelected->currentRow();

    if (row != -1 && row < m_listSelected->count()-1)
    {
        m_listSelected->insertItem(row+1, m_listSelected->takeItem(row));
        m_listSelected->setCurrentRow(row+1);
    }
}

