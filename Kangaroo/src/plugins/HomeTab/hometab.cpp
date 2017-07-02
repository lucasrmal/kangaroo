#include "hometab.h"

#include <QLabel>
#include <QVBoxLayout>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/ledger.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QDebug>

#include "../AccountTreeWidget/accounttreewidget.h"
#include "../TabInterface/centralwidget.h"
#include "ledgertab.h"
#include "../AccountCreateEdit/formeditaccount.h"

using namespace KLib;

HomeTab::HomeTab(QWidget *parent) :
    QWidget(parent),
    m_accountTree(NULL),
    m_currentAccount(NULL)
{
//    QLabel* lblTitle = new QLabel();
//    lblTitle->setText("Summary of Accounts");

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    //m_layout->addWidget(lblTitle);
    setLayout(m_layout);

    //Create menus
    m_accountMenu = ActionManager::instance()->insertMenu(STD_MENUS::MENUBAR,
                                                          STD_MENUS::MENU_MANAGE,
                                                          "Account",
                                                          tr("&Account"));

    m_actOpen = m_accountMenu->menu()->addAction(Core::icon("open-ledger"), tr("Open &Ledger"), this, SLOT(openLedger()));
    m_accountMenu->menu()->addSeparator();
    m_actCreate = m_accountMenu->menu()->addAction(Core::icon("create-bank-account"), tr("Add &Account"), this, SLOT(createAccount()));
    m_actEdit = m_accountMenu->menu()->addAction(Core::icon("edit-bank-account"), tr("&Edit Account"), this, SLOT(editAccount()));
    m_accountMenu->menu()->addSeparator();
    m_actClose = m_accountMenu->menu()->addAction(Core::icon("close-bank-account"), tr("&Close Account"), this, SLOT(closeAccount()));
    m_actReopen = m_accountMenu->menu()->addAction(Core::icon("reopen-bank-account"), tr("Re-&open Account"), this, SLOT(reopenAccount()));
    m_actRemove = m_accountMenu->menu()->addAction(Core::icon("remove-bank-account"), tr("&Remove Account"), this, SLOT(removeAccount()));
    m_accountMenu->menu()->addSeparator();
    m_actShowAll = m_accountMenu->menu()->addAction(tr("&Show All Accounts"));
    m_actShowAll->setCheckable(true);
    m_actShowAll->setChecked(false);
    connect(m_actShowAll, SIGNAL(triggered(bool)), this, SLOT(showAllAccounts(bool)));

    ActionManager::instance()->registerAction(m_actCreate, "Account.Add", QKeySequence("F6"));
    ActionManager::instance()->registerAction(m_actOpen,   "Account.OpenLedger", QKeySequence("F7"));
    ActionManager::instance()->registerAction(m_actEdit,   "Account.Edit");
    ActionManager::instance()->registerAction(m_actClose,  "Account.Close");
    ActionManager::instance()->registerAction(m_actReopen, "Account.Reopen");
    ActionManager::instance()->registerAction(m_actRemove, "Account.Remove");
    ActionManager::instance()->registerAction(m_actShowAll, "Account.ShowAll");

    m_actOpen->setEnabled(false);
    m_actEdit->setEnabled(false);
    m_actClose->setEnabled(false);
    m_actReopen->setEnabled(false);
    m_actRemove->setEnabled(false);

    m_accountMenu->menu()->setEnabled(false);

}

HomeTab::~HomeTab()
{
    qDebug() << "Destroyed";
}

void HomeTab::onLoad()
{
    m_accountMenu->menu()->setEnabled(true);
    m_actCreate->setEnabled(true);
    m_actOpen->setEnabled(false);
    m_actEdit->setEnabled(false);
    m_actClose->setEnabled(false);
    m_actReopen->setEnabled(false);
    m_actRemove->setEnabled(false);
    m_actShowAll->setEnabled(true);

    m_accountTree = new AccountTreeWidget(this);
    connect(m_accountTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openLedger()));
    connect(m_accountTree, SIGNAL(currentAccountChanged(KLib::Account*)), this, SLOT(currentAccountChanged(KLib::Account*)));

    connect(CentralWidget::instance(), SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged()));

    m_layout->addWidget(m_accountTree);
}

void HomeTab::onUnload()
{
    m_accountMenu->menu()->setEnabled(false);
    m_actCreate->setEnabled(false);
    m_actOpen->setEnabled(false);
    m_actEdit->setEnabled(false);
    m_actClose->setEnabled(false);
    m_actReopen->setEnabled(false);
    m_actRemove->setEnabled(false);
    m_actShowAll->setEnabled(false);

    if (m_accountTree)
    {
        m_layout->removeWidget(m_accountTree);
        m_accountTree->deleteLater();
        m_accountTree = NULL;
    }
}

void HomeTab::onCurrentTabChanged()
{
    if (CentralWidget::instance()->currentWidget() == this)
    {
        currentAccountChanged(m_accountTree->currentAccount());
    }
    else
    {
        //See if it is a Ledger
        LedgerTab* tab = qobject_cast<LedgerTab*>(CentralWidget::instance()->currentWidget());

        if (tab)
        {
            currentAccountChanged(tab->account());
            m_actOpen->setEnabled(false);
        }
        else
        {
            m_actOpen->setEnabled(false);
            m_actEdit->setEnabled(false);
            m_actClose->setEnabled(false);
            m_actReopen->setEnabled(false);
            m_actRemove->setEnabled(false);
        }
    }
}

void HomeTab::currentAccountChanged(KLib::Account* a)
{
    if (a)
    {
        m_actOpen->setEnabled(!a->isPlaceholder());
        m_actEdit->setEnabled(true);
        m_actClose->setEnabled(a->isOpen() && !a->isPlaceholder());
        m_actReopen->setEnabled(!a->isOpen() && !a->isPlaceholder());
        m_actRemove->setEnabled(a->childCount() == 0 && a->type() != AccountType::TOPLEVEL && (a->isPlaceholder() || a->ledger()->count() == 0));
    }
    else
    {
        m_actOpen->setEnabled(false);
        m_actEdit->setEnabled(false);
        m_actClose->setEnabled(false);
        m_actReopen->setEnabled(false);
        m_actRemove->setEnabled(false);
    }

    m_currentAccount = a;
}

void HomeTab::resetAccountTree()
{
    m_accountTree->reset();
}

void HomeTab::showAllAccounts(bool _show)
{
    m_accountTree->resetModel(!_show);
}

void HomeTab::createAccount()
{
    FormEditAccount* formCreate = new FormEditAccount(NULL, this);
    formCreate->setParentAccount(m_currentAccount);
    formCreate->exec();
    delete formCreate;
}

void HomeTab::editAccount()
{
    if (!m_currentAccount) return;

    FormEditAccount* formEdit = new FormEditAccount(m_currentAccount, this);
    formEdit->exec();
    delete formEdit;
}

void HomeTab::closeAccount()
{
    if (!m_currentAccount) return;

    if (QMessageBox::question(this,
                              tr("Close Account"),
                              tr("Close account %1 ?").arg(m_currentAccount->name()),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::Yes)
    {
        try
        {
            //LedgerTabManager::instance()->closeLedger(m_currentAccount);
            m_currentAccount->setOpen(false);
            currentAccountChanged(m_currentAccount);
        }
        catch (ModelException e)
        {
            QMessageBox::warning(this,
                                 tr("Close Account"),
                                 tr("An error has occured while closing the account:\n\n%1").arg(e.description()));
        }
    }
}

void HomeTab::reopenAccount()
{
    if (!m_currentAccount) return;

    if (QMessageBox::question(this,
                              tr("Reopen Account"),
                              tr("Reopen account %1 ?").arg(m_currentAccount->name()),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::Yes)
    {

        try
        {
            m_currentAccount->setOpen(true);
            currentAccountChanged(m_currentAccount);
        }
        catch (ModelException e)
        {
            QMessageBox::warning(this,
                                 tr("Reopen Account"),
                                 tr("An error has occured while opening the account:\n\n%1").arg(e.description()));
        }
    }

}

void HomeTab::removeAccount()
{
    if (!m_currentAccount ||
        m_currentAccount->type() == AccountType::TOPLEVEL ||
        (!m_currentAccount->isPlaceholder() && m_currentAccount->ledger()->count()) ||
        m_currentAccount->childCount())
    {
        return;
    }

    if (QMessageBox::question(this,
                              tr("Remove Account"),
                              tr("Remove account %1 ?").arg(m_currentAccount->name()),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::Yes)
    {
        try
        {
            //LedgerTabManager::instance()->closeLedger(m_currentAccount);
            m_currentAccount->parent()->removeChildren(m_currentAccount->id());
            m_currentAccount = NULL;
            currentAccountChanged(m_currentAccount);
        }
        catch (ModelException e)
        {
            QMessageBox::warning(this,
                                 tr("Remove Account"),
                                 tr("An error has occured while removing the account:\n\n%1").arg(e.description()));
        }
    }
}

void HomeTab::openLedger()
{
    if (m_currentAccount)
    {
        if (m_currentAccount->isPlaceholder())
        {
//            QMessageBox::warning(this,
//                                 tr("Open Ledger"),
//                                 tr("This account is a placeholder, it cannot have any ledger."));
            return;
        }

        LedgerTabManager::instance()->openLedger(m_currentAccount);
    }
}
