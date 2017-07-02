#include "incomeexpensetab.h"
#include "accounttreewidget.h"
#include <KangarooLib/model/account.h>
#include <KangarooLib/controller/accountcontroller.h>
#include <QToolBar>
#include <QComboBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QMessageBox>

using namespace KLib;

IncomeExpenseTab::IncomeExpenseTab(QWidget* _parent) :
    AccountViewTab(Account::getTopLevel(), true, AccountTypeFlags::Flag_IncomeExpense, _parent)
{    
    //Date selector
    QWidget* dateSelector = new QWidget(this);
    QGridLayout* dateLayout = new QGridLayout(dateSelector);
    dateLayout->setContentsMargins(0,0,0,0);
    m_cboBalanceSelector = new QComboBox(this);
    m_spinYear = new QSpinBox(this);
    m_cboMonth = new QComboBox(this);

    m_spinYear->setMinimum(1900);
    m_spinYear->setMaximum(9999);
    m_spinYear->setValue(QDate::currentDate().year());

    for (int i = 1; i <= 12; ++i)
        m_cboMonth->addItem(QDate::longMonthName(i));

    m_cboMonth->setCurrentIndex(QDate::currentDate().month()-1);

    m_cboBalanceSelector->addItem(tr("Through Today"));
    m_cboBalanceSelector->addItem(tr("All Time"));
    m_cboBalanceSelector->addItem(tr("Year"));
    m_cboBalanceSelector->addItem(tr("Month"));
    m_cboBalanceSelector->setMaximumWidth(200);

    connect(m_cboBalanceSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeBalanceType(int)));

    connect(m_spinYear, SIGNAL(valueChanged(int)),        SLOT(updateBalanceDates()));
    connect(m_cboMonth, SIGNAL(currentIndexChanged(int)), SLOT(updateBalanceDates()));

    dateLayout->addWidget(m_cboBalanceSelector, 0, 0, 1, 2);
    dateLayout->addWidget(m_spinYear, 1, 0);
    dateLayout->addWidget(m_cboMonth, 1, 1);

    //Create the toolbar
    QToolBar* bottomToolbar = new QToolBar(this);

    QWidget* sep1 = new QWidget();
    sep1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

    bottomToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    bottomToolbar->setIconSize(QSize(24,24));

    bottomToolbar->addAction(action(ActionType::Create));
    bottomToolbar->addAction(action(ActionType::Edit));
    bottomToolbar->addAction(action(ActionType::CloseReopen));
    bottomToolbar->addAction(action(ActionType::Delete));
    bottomToolbar->addAction(action(ActionType::Open));
    bottomToolbar->addWidget(sep1);
    bottomToolbar->addWidget(dateSelector);
    bottomToolbar->addAction(action(ActionType::Find));
//    bottomToolbar->addAction(tr("Convert Dist"), this, SLOT(convertDividend()));
    setToolBar(bottomToolbar);

    onChangeBalanceType(0);
}

//#include <KangarooLib/model/transaction.h>
//#include <KangarooLib/model/investmenttransaction.h>
//#include <KangarooLib/model/ledger.h>
//#include <KangarooLib/ui/widgets/accountselector.h>
//#include <KangarooLib/ui/dialogs/camsegdialog.h>

//void IncomeExpenseTab::convertDividend()
//{
//    if (m_accountTree->currentAccount()
//        && m_accountTree->currentAccount()->type() == AccountType::INCOME
//        && !m_accountTree->currentAccount()->isPlaceholder())
//    {
//        AccountSelector* selector = new AccountSelector(Flag_None,
//                                                 AccountTypeFlags::Flag_Investment,
//                                                 Constants::NO_ID);

//        CAMSEGDialog* d = new CAMSEGDialog(selector, CAMSEGDialog::DialogWithoutPicture, CAMSEGDialog::OkCancelButtons, this);
//        d->setBothTitles(tr("Convert Distribution?"));

//        if (d->exec() == QDialog::Accepted && selector->currentAccount())
//        {

//            Account* a = m_accountTree->currentAccount();
//            QList<Transaction*> toRemove;

//            auto transactions = a->ledger()->transactions();

//            for (auto i = transactions->begin(); i != transactions->end(); ++i)
//            {
//                Transaction* t = *i;

//                if (!qobject_cast<InvestmentTransaction*>(t)) //If not already an investment transaction
//                {
//                    QList<InvestmentSplitType> types;
//                    QList<Transaction::Split> splits;

//                    for (auto s : t->splits())
//                    {
//                        if (s.idAccount == a->id()) //Dividend source
//                        {
//                            types << InvestmentSplitType::DistributionSource;
//                            splits << s;
//                        }
//                        else if (Account::generalType(Account::getTopLevel()->account(s.idAccount)->type()) == AccountType::ASSET)
//                        {
//                            types << InvestmentSplitType::DistributionDest;
//                            splits << s;
//                        }
//                        else if (Account::getTopLevel()->account(s.idAccount)->type() == AccountType::EXPENSE)
//                        {
//                            types << InvestmentSplitType::Tax;
//                            splits << s;
//                        }
//                    }

//                    //Check if balances
//                    if (Transaction::splitsBalance(splits))
//                    {
//                        //Convert to investment transaction
//                        InvestmentTransaction* itr = new InvestmentTransaction;
//                        itr->setDate(t->date());
//                        itr->setClearedStatus(t->clearedStatus());
//                        itr->setMemo(t->memo());
//                        itr->setFlagged(t->isFlagged());
//                        itr->setNo(t->no());
//                        itr->setNote(t->note());

//                        itr->makeDivDist(InvestmentAction::Distribution,
//                                         selector->currentAccount()->id(),
//                                         splits,
//                                         types);
//                        LedgerManager::instance()->addTransaction(itr);
//                        toRemove.append(t);
//                    }
//                }
//            }

//            //Now remove the old plain transactions
//            for (Transaction* t : toRemove)
//            {
//                LedgerManager::instance()->removeTransaction(t->id());
//            }
//        }

//        delete d;
//    }
//}

void IncomeExpenseTab::onChangeBalanceType(int _index)
{
    switch ((BalanceType) _index)
    {
    case BalanceType::ThroughToday:
    case BalanceType::AllTime:
        m_cboMonth->setEnabled(false);
        m_spinYear->setEnabled(false);
        break;

    case BalanceType::Year:
        m_cboMonth->setEnabled(false);
        m_spinYear->setEnabled(true);
        m_spinYear->setFocus();
        break;

    case BalanceType::Month:
        m_cboMonth->setEnabled(true);
        m_spinYear->setEnabled(true);
        m_cboMonth->setFocus();
        break;
    }

    updateBalanceDates();
}

void IncomeExpenseTab::updateBalanceDates()
{
    QDate begin, end;

    switch ((BalanceType) m_cboBalanceSelector->currentIndex())
    {
    case BalanceType::ThroughToday:
        end = QDate::currentDate();
        break;

    case BalanceType::Year:
        begin = QDate(m_spinYear->value(), 1, 1);
        end = QDate(m_spinYear->value(), 12, 31);
        break;

    case BalanceType::Month:
        begin = QDate(m_spinYear->value(), m_cboMonth->currentIndex()+1, 1);
        end = QDate(m_spinYear->value(), m_cboMonth->currentIndex()+1, begin.daysInMonth());
        break;

    case BalanceType::AllTime: //Nothing to do :-)
        break;
    }

    m_accountTree->controller()->setBalancesBetween(begin, end);

}


