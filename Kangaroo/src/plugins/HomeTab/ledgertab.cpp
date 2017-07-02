#include "ledgertab.h"
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/ledger.h>
#include <KangarooLib/model/security.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/picturemanager.h>
#include <KangarooLib/model/pricemanager.h>
#include <KangarooLib/ui/widgets/ledgerwidget.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/dialogs/formpicturemanager.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QToolBar>
#include <QKeySequence>
#include <QStatusBar>
#include "../TabInterface/centralwidget.h"
#include "../AccountCreateEdit/formeditaccount.h"

#define TABNAME(acc) QString("Account_%1").arg(acc->id())

using namespace KLib;

LedgerTabManager* LedgerTabManager::m_instance = NULL;

LedgerTab::LedgerTab(Account *_account, QWidget *parent) :
    QWidget(parent),
    m_account(_account)
{
    LedgerTabManager::instance()->registerLedger(this);

    if (!_account->ledger())
    {
        return;
    }

    m_ledgerWidget = new LedgerWidget(_account, this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* layoutTop = new QHBoxLayout();
    QVBoxLayout* layoutTopL = new QVBoxLayout();
    QVBoxLayout* layoutTopR = new QVBoxLayout();

    QVBoxLayout* layoutBottom = new QVBoxLayout();

    layout->setContentsMargins(0,0,0,0);
    layoutTop->setContentsMargins(6,6,6,0);

    m_lblPic = new QPushButton(this);
    m_lblAccountName = new QLabel(this);
    m_lblAccountType = new QLabel(this);
    m_lblAccountBalance = new QLabel(this);
    m_lblAccountValue = new QLabel(this);
    //m_btnEditAccount = new QPushButton(Core::icon("edit-bank-account"), tr("Edit"), this);

    m_lblPic->setFixedSize(PictureManager::THUMBNAIL_SIZE);

    QWidget* sep1 = new QWidget();
    sep1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    QWidget* sep2 = new QWidget();
    sep2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

    QToolBar* bottomToolbar = new QToolBar(this);
    bottomToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    bottomToolbar->setIconSize(QSize(24,24));
    bottomToolbar->addAction(LedgerTabManager::instance()->action_new());
    bottomToolbar->addAction(LedgerTabManager::instance()->action_edit());
    bottomToolbar->addAction(LedgerTabManager::instance()->action_duplicate());
    bottomToolbar->addAction(LedgerTabManager::instance()->action_remove());
    bottomToolbar->addWidget(sep1);
    bottomToolbar->addAction(LedgerTabManager::instance()->action_enter());
    bottomToolbar->addAction(LedgerTabManager::instance()->action_discard());
    bottomToolbar->addAction(LedgerTabManager::instance()->action_splits());
    bottomToolbar->addAction(LedgerTabManager::instance()->action_attachments());
    bottomToolbar->addWidget(sep2);
    bottomToolbar->addAction(LedgerTabManager::instance()->action_find());

    layoutBottom->addWidget(bottomToolbar);


    QFont f;
    f.setPointSize(13);
    f.setBold(true);
    m_lblAccountName->setFont(f);

    f.setPointSize(11);
    m_lblAccountType->setFont(f);

    f.setPointSize(10);
    m_lblAccountBalance->setFont(f);
    m_lblAccountValue->setFont(f);

    m_lblAccountBalance->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
    //m_btnEditAccount->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    //m_btnEditAccount->setFlat(true);
    m_lblPic->setFlat(true);
    m_lblPic->setIconSize(PictureManager::THUMBNAIL_SIZE);

    m_lblAccountBalance->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_lblAccountValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layoutTopL->addWidget(m_lblAccountName);
    layoutTopL->addWidget(m_lblAccountType);
    layoutTopR->addWidget(m_lblAccountBalance);
    layoutTopR->addWidget(m_lblAccountValue);
    //layoutTopR->addWidget(m_btnEditAccount);

    layoutTop->addWidget(m_lblPic);
    layoutTop->addLayout(layoutTopL);
    layoutTop->addStretch(2);
    layoutTop->addLayout(layoutTopR);

    layout->addLayout(layoutTop);
    layout->addWidget(m_ledgerWidget);
    layout->addLayout(layoutBottom);
    setLayout(layout);

    updateAccountInfos();
    updateAccountBalance();


    connect(m_ledgerWidget, SIGNAL(editNewStarted()),   this, SIGNAL(editNewStarted()));
    connect(m_ledgerWidget, SIGNAL(editStarted()),      this, SIGNAL(editStarted()));
    connect(m_ledgerWidget, SIGNAL(changesDiscarded()), this, SIGNAL(changesDiscarded()));
    connect(m_ledgerWidget, SIGNAL(changesSaved()),     this, SIGNAL(changesSaved()));
    connect(m_ledgerWidget, SIGNAL(selectedRowsTotalChanged(KLib::Amount)), this, SLOT(updateBalance(KLib::Amount)));
    connect(m_ledgerWidget, SIGNAL(selectedRowsChanged(QList<int>)), this, SIGNAL(selectedRowsChanged(QList<int>)));

    connect(m_account, SIGNAL(accountModified(KLib::Account*)), this, SLOT(updateAccountInfos()));
    connect(m_account->ledger(), SIGNAL(modified()), this, SLOT(updateAccountBalance()));
    connect(PriceManager::instance(), &PriceManager::lastRateModified,
            this, &LedgerTab::updateAccountBalance);

    if (m_account->type() == AccountType::INVESTMENT)
    {
        //We cannot connect directly to the security in case it changes...
        connect(SecurityManager::instance(), &SecurityManager::modified,
                this, &LedgerTab::updateAccountInfos);
    }
    else if (m_account->type() == AccountType::TRADING && m_account->idSecurity() != Constants::NO_ID)
    {
        //Security will never change, so we're safe.
        connect(SecurityManager::instance()->get(m_account->idSecurity()), &Security::modified,
                this, &LedgerTab::updateAccountInfos);
    }
    else if (m_account->type() == AccountType::CREDITCARD)
    {
        //In case the credit limit is modified...
        connect(m_account, SIGNAL(accountModified(KLib::Account*)), this, SLOT(updateAccountBalance()));
    }

    //connect(m_btnEditAccount, &QPushButton::clicked, this, &LedgerTab::editAccount);
    connect(m_lblPic, &QPushButton::clicked, this, &LedgerTab::selectAccountPicture);

}

void LedgerTab::updateAccountInfos()
{
    m_lblAccountName->setText(m_account->name());
    m_lblAccountType->setText(Account::typeToString(m_account->type()));
    m_lblPic->setIcon(QIcon(m_account->defaultPicture(true)));
}

void LedgerTab::updateAccountBalance()
{
    Amount balance = m_account->balanceToday();

    if (m_account->type() == AccountType::INVESTMENT)
    {
        Security* sec = SecurityManager::instance()->get(m_account->idSecurity());
        Currency* cur = CurrencyManager::instance()->get(sec->currency());

        m_lblAccountBalance->setText(tr("Balance: %1 shares").arg(QString::number(balance.toDouble())));
        m_lblAccountValue->setText(tr("Value: %1")
                                   .arg(cur->formatAmount(balance
                                                        * PriceManager::instance()->rate(m_account->idSecurity(),
                                                                                         sec->currency()))));
    }
    else
    {
        m_lblAccountBalance->setText(tr("Balance: %1").arg(m_account->formatAmount(balance)));
    }

    if (m_account->type() == AccountType::CREDITCARD && m_account->properties()->contains("creditlimit"))
    {
        Amount limit = Amount::fromQVariant(m_account->properties()->get("creditlimit"));
        m_lblAccountValue->setText(tr("Available Credit: %1")
                                   .arg(m_account->formatAmount(limit-balance)));
    }
}

void LedgerTab::editAccount()
{
    FormEditAccount* formEdit = new FormEditAccount(m_account, this);
    formEdit->exec();
    delete formEdit;
}

void LedgerTab::selectAccountPicture()
{
    FormPictureManager picMan(m_account->idPicture(), FormPictureManager::AllowNoPicture, this);

    if (picMan.exec() == QDialog::Accepted)
    {
        m_account->setIdPicture(picMan.currentPictureId());
    }
}

void LedgerTab::updateBalance(const Amount& _amount)
{
    Core::instance()->mainWindow()->statusBar()->showMessage(tr("Selected total: %1")
                                                             .arg(m_account->formatAmount(_amount)));
}

LedgerTabManager* LedgerTabManager::instance()
{
    if (!m_instance)
        m_instance = new LedgerTabManager();

    return m_instance;
}

LedgerTabManager::LedgerTabManager() :
    m_currentTab(NULL)
{

    m_icnEnter = Core::icon("enter-modifications");
    m_icnEnterSchedule = Core::icon("enter-schedule");
    //Create menus
    m_transacMenu = ActionManager::instance()->insertMenu(STD_MENUS::MENUBAR,
                                                          STD_MENUS::MENU_MANAGE,
                                                          "Transaction",
                                                          tr("&Transaction"));

    m_actNew = m_transacMenu->menu()->addAction(Core::icon("new-row"),
                                                tr("&New"), this, SLOT(newTransaction()));
    m_actEdit = m_transacMenu->menu()->addAction(Core::icon("edit"),
                                                 tr("&Edit"), this, SLOT(editTransaction()));
    m_actDuplicate = m_transacMenu->menu()->addAction(Core::icon("copy"),
                                                    tr("D&uplicate"), this, SLOT(duplicateTransaction()));
    m_actRemove = m_transacMenu->menu()->addAction(Core::icon("trash"),
                                                 tr("&Delete"), this, SLOT(deleteTransactions()));
    m_transacMenu->menu()->addSeparator();
    m_actEnter = m_transacMenu->menu()->addAction(m_icnEnter,
                                                  tr("En&ter"), this, SLOT(enterTransaction()));
    m_actSplits = m_transacMenu->menu()->addAction(Core::icon("split"),
                                                    tr("&Splits"), this, SLOT(editSplits()));
    m_actAttachments = m_transacMenu->menu()->addAction(Core::icon("attachment"),
                                                    tr("&Attachments"), this, SLOT(editAttachments()));
    m_actDiscard = m_transacMenu->menu()->addAction(Core::icon("erase"),
                                                    tr("&Cancel Edit"), this, SLOT(discardTransaction()));
    m_transacMenu->menu()->addSeparator();
    m_actFind = m_transacMenu->menu()->addAction(Core::icon("find"),
                                                    tr("Searc&h"), this, SLOT(findTransactions()));


    ActionManager::instance()->registerAction(m_actNew, "Transaction.New", QKeySequence("Ctrl+N"));
    ActionManager::instance()->registerAction(m_actEdit, "Transaction.Edit");
    ActionManager::instance()->registerAction(m_actRemove, "Transaction.Remove", QKeySequence("Ctrl+D"));
    ActionManager::instance()->registerAction(m_actEnter, "Transaction.Enter", QKeySequence("Ctrl+Enter"));
    ActionManager::instance()->registerAction(m_actSplits, "Transaction.EditSplits", QKeySequence("Ctrl+L"));
    ActionManager::instance()->registerAction(m_actAttachments, "Transaction.Attachments");
    ActionManager::instance()->registerAction(m_actDiscard, "Transaction.Discard", QKeySequence("Esc"));
    ActionManager::instance()->registerAction(m_actDuplicate, "Transaction.Duplicate", QKeySequence("Ctrl+U"));
    ActionManager::instance()->registerAction(m_actFind, "Transaction.Search", QKeySequence("Ctrl+F"));

    m_transacMenu->menu()->setEnabled(false);

    m_actNew->setEnabled(false);
    m_actEdit->setEnabled(false);
    m_actRemove->setEnabled(false);
    m_actEnter->setEnabled(false);
    m_actSplits->setEnabled(false);
    m_actAttachments->setEnabled(false);
    m_actDiscard->setEnabled(false);
    m_actDuplicate->setEnabled(false);
    m_actFind->setEnabled(false);

    //Connections to Account changes
    connect(Account::getTopLevel(), SIGNAL(accountModified(KLib::Account*)),
            this, SLOT(onAccountModified(KLib::Account*)));
    connect(Account::getTopLevel(), SIGNAL(accountRemoved(KLib::Account*)),
            this, SLOT(onAccountRemoved(KLib::Account*)));
}

void LedgerTabManager::onLoad()
{
    m_transacMenu->menu()->setEnabled(true);

    onCurrentTabChanged();
    connect(CentralWidget::instance(), SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged()));
}

void LedgerTabManager::onUnload()
{
    m_transacMenu->menu()->setEnabled(false);

    m_actNew->setEnabled(false);
    m_actEdit->setEnabled(false);
    m_actRemove->setEnabled(false);
    m_actEnter->setEnabled(false);
    m_actSplits->setEnabled(false);
    m_actAttachments->setEnabled(false);
    m_actDiscard->setEnabled(false);
    m_actDuplicate->setEnabled(false);
    m_actFind->setEnabled(false);

    for (LedgerTab* t : m_tabs)
    {
        t->deleteLater();
    }

    m_tabs.clear();
    m_tabIndex.clear();
    m_currentTab = NULL;

}

void LedgerTabManager::checkActions()
{
    bool enable = m_currentTab->account()->isOpen();

    QList<int> rows = m_currentTab->m_ledgerWidget->selectedRows();

    m_actNew->setEnabled(enable);
    m_actEdit->setEnabled(enable
                          && rows.count() == 1
                          && !m_currentTab->m_ledgerWidget->rowIsLocked(rows.first()));
    m_actRemove->setEnabled(enable &&
                            (rows.count() > 1
                             || (rows.count() == 1
                                && rows.first() != m_currentTab->m_ledgerWidget->newTransactionRow())));
    m_actDuplicate->setEnabled(enable
                               && rows.count() == 1
                               && rows.first() != m_currentTab->m_ledgerWidget->newTransactionRow()
                               && !m_currentTab->m_ledgerWidget->rowIsLocked(rows.first()));
    m_actSplits->setEnabled(rows.count() == 1);
    m_actAttachments->setEnabled(rows.count() == 1);
    m_actEnter->setEnabled(enable
                           && rows.count() == 1
                           && m_currentTab->m_ledgerWidget->rowIsSchedule(rows.first()));
    m_actDiscard->setEnabled(false);
    m_actFind->setEnabled(true);

    if (enable && rows.count() == 1 && m_currentTab->m_ledgerWidget->rowIsSchedule(rows.first()))
    {
        m_actEdit->setIcon(m_icnEnterSchedule);
    }
    else
    {
        m_actEdit->setIcon(m_icnEnter);
    }
}

void LedgerTabManager::onCurrentTabChanged()
{
    LedgerTab* tab = static_cast<LedgerTab*>(CentralWidget::instance()->currentWidget());

    Core::instance()->mainWindow()->statusBar()->clearMessage();

    if (m_tabs.contains(tab))
    {
        m_currentTab = tab;
        checkActions();
    }
    else
    {
        m_currentTab = NULL;

        m_actNew->setEnabled(false);
        m_actEdit->setEnabled(false);
        m_actRemove->setEnabled(false);
        m_actSplits->setEnabled(false);
        m_actAttachments->setEnabled(false);
        m_actDuplicate->setEnabled(false);
        m_actFind->setEnabled(false);
        m_actEnter->setEnabled(false);
        m_actDiscard->setEnabled(false);
    }

}

void LedgerTabManager::onEditStarted()
{
    LedgerTab* source = static_cast<LedgerTab*>(sender());

    if (source && source == m_currentTab)
    {
        if (!m_currentTab->account()->isOpen())
            return;

        m_actNew->setEnabled(false);
        m_actEdit->setEnabled(false);
        m_actEnter->setEnabled(true);
        m_actDiscard->setEnabled(true);
        m_actRemove->setEnabled(true);
        m_actDuplicate->setEnabled(false);
        m_actAttachments->setEnabled(true);
        m_actSplits->setEnabled(true);

        m_actEdit->setIcon(m_icnEnter);
    }
}

void LedgerTabManager::onEditNewStarted()
{
    LedgerTab* source = static_cast<LedgerTab*>(sender());

    if (source && source == m_currentTab)
    {
        if (!m_currentTab->account()->isOpen())
            return;

        m_actNew->setEnabled(false);
        m_actEdit->setEnabled(false);
        m_actEnter->setEnabled(true);
        m_actDiscard->setEnabled(true);
        m_actRemove->setEnabled(false);
        m_actDuplicate->setEnabled(false);
        m_actAttachments->setEnabled(true);
        m_actSplits->setEnabled(true);

        m_actEdit->setIcon(m_icnEnter);
    }
}

void LedgerTabManager::onSelectedRowsChanged(const QList<int>& _rows)
{
    Q_UNUSED(_rows)
    LedgerTab* source = static_cast<LedgerTab*>(sender());

    if (source && source == m_currentTab)
    {
        checkActions();
    }
}

void LedgerTabManager::onAccountModified(KLib::Account* _a)
{
    if (_a->isPlaceholder() && m_tabIndex.contains(_a->id()))
    {
        removeTab(m_tabIndex[_a->id()]);
    }
    else if (!_a->isOpen() && m_tabIndex.contains(_a->id()))
    {
        m_actNew->setEnabled(false);
        m_actEdit->setEnabled(false);
        m_actRemove->setEnabled(false);
        m_actDuplicate->setEnabled(false);

        LedgerTab* tab = m_tabIndex[_a->id()];
        if (tab->m_ledgerWidget->inEdit())
        {
            tab->m_ledgerWidget->discardEdit();
        }
    }

}

void LedgerTabManager::onAccountRemoved(KLib::Account* _a)
{
    if (m_tabIndex.contains(_a->id()))
    {
        removeTab(m_tabIndex[_a->id()]);
    }
}

void LedgerTabManager::onChangesDiscarded()
{
    LedgerTab* source = static_cast<LedgerTab*>(sender());

    if (source && source == m_currentTab)
    {
        checkActions();
    }
}

void LedgerTabManager::onChangedSaved()
{
    LedgerTab* source = static_cast<LedgerTab*>(sender());

    if (source && source == m_currentTab)
    {
        checkActions();
    }
}

bool LedgerTabManager::askToContinue()
{
    int ans = QMessageBox::question(m_currentTab,
                                    tr("Save Changes"),
                                    tr("The current transaction has unsaved changes. Save it?"),
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                    QMessageBox::Yes);

    switch (ans)
    {
    case QMessageBox::Yes:
        m_currentTab->m_ledgerWidget->submitEdit();
    case QMessageBox::No:
        return true;
    default:
        return false;
    }
}

void LedgerTabManager::newTransaction()
{
    if (m_currentTab)
    {
        if (m_currentTab->m_ledgerWidget->inEdit() && !askToContinue())
        {
            return;
        }
        m_currentTab->m_ledgerWidget->editNew();
    }
}

void LedgerTabManager::editTransaction()
{
    if (m_currentTab)
    {
        if (m_currentTab->m_ledgerWidget->inEdit() && !askToContinue())
        {
            return;
        }

        m_currentTab->m_ledgerWidget->editCurrent();
    }
}

void LedgerTabManager::deleteTransactions()
{
    if (m_currentTab->m_ledgerWidget->inNewEdit())
    {
        discardTransaction();
    }
    else
    {
        QList<int> rows = m_currentTab->m_ledgerWidget->selectedRows();
        int num = rows.count();

        for (int i : rows)
        {
            if (i == m_currentTab->m_ledgerWidget->newTransactionRow())
            {
                --num;
                break;
            }
        }

        int answer;

        if (num == 0)
        {
            return;
        }
        else if (num == 1)
        {
            answer = QMessageBox::question(m_currentTab,
                                                       tr("Delete Transaction"),
                                                       tr("Delete this transaction?"),
                                                       QMessageBox::Yes | QMessageBox::No,
                                                       QMessageBox::No);
        }
        else
        {
            answer = QMessageBox::question(m_currentTab,
                                                       tr("Delete Transactions"),
                                                       tr("Delete these %1 transactions?").arg(num),
                                                       QMessageBox::Yes | QMessageBox::No,
                                                       QMessageBox::No);
        }

        if (answer == QMessageBox::No)
            return;

        m_currentTab->m_ledgerWidget->deleteSelected();
    }
}

void LedgerTabManager::removeTab(QWidget* _tab)
{
    LedgerTab* tab = static_cast<LedgerTab*>(_tab);

    if (tab && m_tabs.contains(tab))
    {
        closeLedger(tab->m_account);
        m_tabs.remove(tab);
        m_tabIndex.remove(tab->m_account->id());
        tab->deleteLater();
    }
}

void LedgerTabManager::enterTransaction()
{
    if (m_currentTab)
    {
        if (m_currentTab->m_ledgerWidget->inEdit())
        {
            m_currentTab->m_ledgerWidget->submitEdit();
        }
        else //Must be entering a schedule
        {
            m_currentTab->m_ledgerWidget->enterSchedule();
        }
    }
}

void LedgerTabManager::discardTransaction()
{
    if (m_currentTab)
    {
        m_currentTab->m_ledgerWidget->discardEdit();
    }
}

void LedgerTabManager::duplicateTransaction()
{
    if (m_currentTab)
    {
        m_currentTab->m_ledgerWidget->duplicateCurrent();
    }
}
void LedgerTabManager::findTransactions()
{

}

void LedgerTabManager::editSplits()
{
    if (m_currentTab)
    {
        m_currentTab->m_ledgerWidget->editSplits();
    }
}

void LedgerTabManager::editAttachments()
{
    if (m_currentTab)
    {
    }
}

void LedgerTabManager::openLedger(Account* _a)
{
    if (_a->isPlaceholder())
        return;

    if (CentralWidget::instance()->hasTab(TABNAME(_a)))
    {
        CentralWidget::instance()->setFocus(TABNAME(_a));
    }
    else
    {
        CentralWidget::instance()->addRegisteredTab(new LedgerTab(_a),
                                                    tr("Ledger: %1").arg(_a->name()),
                                                    TABNAME(_a),
                                                    true);
        CentralWidget::instance()->setFocus(TABNAME(_a));
    }
}

void LedgerTabManager::closeLedger(Account* _a)
{
    if (CentralWidget::instance()->hasTab(TABNAME(_a)))
    {
        CentralWidget::instance()->closeTab(TABNAME(_a));
    }
}

void LedgerTabManager::registerLedger(LedgerTab* _tab)
{
    m_tabIndex[_tab->m_account->id()] = _tab;
    m_tabs.insert(_tab);

    connect(_tab, SIGNAL(changesDiscarded()), this, SLOT(onChangesDiscarded()));
    connect(_tab, SIGNAL(changesSaved()),     this, SLOT(onChangedSaved()));
    connect(_tab, SIGNAL(editStarted()),      this, SLOT(onEditStarted()));
    connect(_tab, SIGNAL(editNewStarted()),   this, SLOT(onEditNewStarted()));
    connect(_tab, SIGNAL(selectedRowsChanged(QList<int>)),   this, SLOT(onSelectedRowsChanged(QList<int>)));
}




