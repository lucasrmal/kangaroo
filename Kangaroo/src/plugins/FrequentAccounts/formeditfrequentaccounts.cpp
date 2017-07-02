#include "formeditfrequentaccounts.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/properties.h>

using namespace KLib;

FormEditFrequentAccounts::FormEditFrequentAccounts(QWidget *parent) :
    CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent)
{
    setTitle(tr("Frequent Accounts"));
    setWindowTitle(title());
    setPicture(Core::pixmap("favorites"));

    m_listAll = new QListWidget(this);
    m_listSelected = new QListWidget(this);

    m_btnSelect = new QPushButton(Core::icon("go-right"), "", this);
    m_btnUnselect = new QPushButton(Core::icon("go-left"), "", this);
    m_btnMoveUp = new QPushButton(Core::icon("go-up"), "", this);
    m_btnMoveDown = new QPushButton(Core::icon("go-down"), "", this);

    connect(m_btnSelect, SIGNAL(clicked()), this, SLOT(select()));
    connect(m_btnUnselect, SIGNAL(clicked()), this, SLOT(unselect()));
    connect(m_btnMoveUp, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(m_btnMoveDown, SIGNAL(clicked()), this, SLOT(moveDown()));

    QVBoxLayout* buttonLayout = new QVBoxLayout();
    buttonLayout->addStretch(5);
    buttonLayout->addWidget(m_btnSelect);
    buttonLayout->addWidget(m_btnUnselect);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(m_btnMoveUp);
    buttonLayout->addWidget(m_btnMoveDown);
    buttonLayout->addStretch(5);

    QLabel* lblAll = new QLabel(tr("All Accounts"), this);
    QLabel* lblSelected = new QLabel(tr("Frequent Accounts"), this);

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

    loadAccounts();

    connect(m_listAll, SIGNAL(currentRowChanged(int)), this, SLOT(onRowChanged_All()));
    connect(m_listSelected, SIGNAL(currentRowChanged(int)), this, SLOT(onRowChanged_Selected()));

    onRowChanged_All();
    onRowChanged_Selected();
}

int forder(Account* a)
{
    return a->properties()->contains("frequent-order") ? a->properties()->get("frequent-order").toInt()
                                                       : 0;
}

void FormEditFrequentAccounts::accept()
{
    // Save the changes
    for (int i = 0; i < m_listAll->count(); ++i)
    {
        Account* a = Account::getTopLevel()->account(m_listAll->item(i)->data(Qt::UserRole).toInt());
        a->properties()->set("frequent", "false");
    }

    for (int i = 0; i < m_listSelected->count(); ++i)
    {
        Account* a = Account::getTopLevel()->account(m_listSelected->item(i)->data(Qt::UserRole).toInt());
        a->properties()->set("frequent", "true");
        a->properties()->set("frequent-order", i);
    }

    done(QDialog::Accepted);
}

void FormEditFrequentAccounts::onRowChanged_All()
{
    m_btnSelect->setEnabled(m_listAll->currentRow()>= 0);
}

void FormEditFrequentAccounts::onRowChanged_Selected()
{
    m_btnMoveDown->setEnabled(m_listSelected->currentRow() != -1 && m_listSelected->currentRow() < m_listSelected->count()-1);
    m_btnMoveUp->setEnabled(m_listSelected->currentRow() > 0);
    m_btnUnselect->setEnabled(m_listSelected->currentRow()>= 0);
}

void FormEditFrequentAccounts::select()
{
    if (m_listAll->currentRow()!= -1)
    {
        m_listSelected->addItem(m_listAll->takeItem(m_listAll->currentRow()));
    }
}

void FormEditFrequentAccounts::unselect()
{
    if (m_listSelected->currentRow()!= -1)
    {
        m_listAll->addItem(m_listSelected->takeItem(m_listSelected->currentRow()));
        m_listAll->sortItems();
    }

}

void FormEditFrequentAccounts::moveUp()
{
    int row = m_listSelected->currentRow();

    if (row > 0)
    {
        m_listSelected->insertItem(row-1, m_listSelected->takeItem(row));
        m_listSelected->setCurrentRow(row-1);
    }
}

void FormEditFrequentAccounts::moveDown()
{
    int row = m_listSelected->currentRow();

    if (row != -1 && row < m_listSelected->count()-1)
    {
        m_listSelected->insertItem(row+1, m_listSelected->takeItem(row));
        m_listSelected->setCurrentRow(row+1);
    }
}

void FormEditFrequentAccounts::loadAccounts()
{
    QList<Account*> selected;
    QList<Account*> others;

    for (Account* a : Account::getTopLevel()->accounts())
    {
        if (a->isOpen() && !a->isPlaceholder())
        {
            if (a->properties()->contains("frequent") && a->properties()->get("frequent").toString() == "true")
            {
                selected << a;
            }
            else
            {
                others << a;
            }
        }
    }

    std::sort(others.begin(), others.end(),
        [](Account* a, Account* b)
        {
            return a->name().compare(b->name(), Qt::CaseInsensitive) < 0;
        });


    std::sort(selected.begin(), selected.end(),
        [](Account* a, Account* b)
        {
            return forder(a) < forder(b);
        });

    for (Account* a : selected)
    {
        QListWidgetItem* i = new QListWidgetItem(Core::icon(Account::typeToIcon(a->type())),
                                                 a->name(),
                                                 m_listSelected);
        i->setData(Qt::UserRole, a->id());
    }

    for (Account* a : others)
    {
        QListWidgetItem* i = new QListWidgetItem(Core::icon(Account::typeToIcon(a->type())),
                                                 a->name(),
                                                 m_listAll);
        i->setData(Qt::UserRole, a->id());
    }
}



