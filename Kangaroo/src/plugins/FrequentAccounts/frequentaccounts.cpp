#include "frequentaccounts.h"
#include "../HomeTab/ledgertab.h"

#include <QVBoxLayout>
#include <QToolButton>
#include <QSignalMapper>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/properties.h>

using namespace KLib;

FrequentAccounts::FrequentAccounts(QWidget *parent) :
    QDockWidget(tr("Frequent Accounts"), parent),
    m_central(NULL)
{
    m_mapper = new QSignalMapper(this);
    setObjectName("FrequentAccounts");
    reloadAccounts();

    connect(m_mapper, SIGNAL(mapped(int)), this, SLOT(openAccount(int)));

    connect(Account::getTopLevel(), SIGNAL(accountAdded(KLib::Account*)),
            this, SLOT(reloadAccounts()));
    connect(Account::getTopLevel(), SIGNAL(accountRemoved(KLib::Account*)),
            this, SLOT(reloadAccounts()));
}

void FrequentAccounts::openAccount(int _id)
{
    Account* a = Account::getTopLevel()->account(_id);

    if (a->isOpen() && !a->isPlaceholder())
        LedgerTabManager::instance()->openLedger(a);
}

void FrequentAccounts::reloadAccounts()
{
    //Clear the old buttons
    if(m_central) m_central->deleteLater();
    m_central = new QWidget(this);
    m_layout = new QVBoxLayout(m_central);
    setWidget(m_central);
    m_central->setLayout(m_layout);

//    for (QToolButton* b : m_buttons)
//    {
//        m_layout->removeWidget(b);
//        m_mapper->removeMappings(static_cast<QObject*>(b));
//        b->deleteLater();
//    }

//    while (!m_layout->isEmpty())
//    {
//        QLayoutItem* i = m_layout->itemAt(0);
//        m_layout->removeItem(i);
//        delete i;
//    }

//    m_buttons.clear();

    QMultiMap<int, Account*> accounts;

    for (Account* a : Account::getTopLevel()->accounts())
    {
        if (a && a->properties()->contains("frequent") && a->properties()->get("frequent").toString() == "true")
        {
            accounts.insert(a->properties()->get("frequent-order").toInt(),
                            a);
        }
    }

    int buttonsHeight;

    for (Account* a : accounts)
    {
        QToolButton* button = new QToolButton(m_central);
        button->setIcon(Core::icon(Account::typeToIcon(a->type())));
        button->setText(a->name());
        button->setAutoRaise(true);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        //button->setMaximumWidth(200);
        m_layout->addWidget(button);
        buttonsHeight += button->height();

        connect(button, SIGNAL(clicked()), m_mapper, SLOT(map()));
        m_mapper->setMapping(button, a->id());
//        m_buttons.append(button);

    }

    m_layout->addStretch(1);
    m_central->setMaximumHeight(buttonsHeight * 1.2);
    m_central->setMaximumWidth(230);
}
