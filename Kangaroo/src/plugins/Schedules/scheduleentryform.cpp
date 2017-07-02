#include "scheduleentryform.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/widgets/splitswidget.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/model/payee.h>
#include <KangarooLib/model/schedule.h>

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

using namespace KLib;

ScheduleEntryForm::ScheduleEntryForm(const QList<Schedule*>& _schedules,
                                     const QList<QDate>& _dates,
                                     QWidget *parent) :
    CAMSEGDialog(DialogWithPicture,
                 CloseButton,
                 parent),
    m_schedules(_schedules)
{
    if (m_schedules.count() == 0                    //Need to have at least one schedule...
        || m_schedules.count() != _dates.count())
    {
        return;
    }

    //-------------- Load transactions ----------------
    for (int i = 0; i < m_schedules.count(); ++i)
    {
        Schedule* s = m_schedules[i];
        TempTransaction t;
        t.memo = s->transaction()->memo();
        t.payee = s->transaction()->idPayee() == Constants::NO_ID ? "" :
                                                                    PayeeManager::instance()
                                                                    ->get(s->transaction()->idPayee())->name();
        t.date = _dates[i];
        t.splits = s->transaction()->splits();
        m_transactions << t;
    }

    //------------------- Load UI ---------------------

    setPicture(Core::pixmap("schedule"));
    setBothTitles(tr("Scheduled Transactions"));

    QLabel* lblInfos = new QLabel(this);
    lblInfos->setText(tr("The following scheduled transaction are due. You can make changes "
                         "to the current instances, and click \"Enter\" to enter them."));
    lblInfos->setWordWrap(true);

    m_frmSchedule = new QGroupBox(this);
    m_txtNo = new QLineEdit(this);
    m_txtMemo = new QLineEdit(this);
    m_cboPayee = new QComboBox(this);
    m_dteDate = new QDateEdit(this);
    m_splitsWidget = new SplitsWidget(m_splits, false, this);

    QLabel* lblNo = new QLabel(tr("&No:"), this);
    QLabel* lblMemo = new QLabel(tr("&Memo:"), this);
    QLabel* lblPayee = new QLabel(tr("P&ayee:"), this);
    QLabel* lblDate = new QLabel(tr("&Date:"), this);

    Core::setupDateEdit(m_dteDate);

    QGridLayout* layoutSchedule = new QGridLayout(m_frmSchedule);
    layoutSchedule->addWidget(lblNo,        0, 0);
    layoutSchedule->addWidget(m_txtNo,      0, 1);
    layoutSchedule->addWidget(lblDate,      0, 2);
    layoutSchedule->addWidget(m_dteDate,    0, 3);
    layoutSchedule->addWidget(lblPayee,     0, 4);
    layoutSchedule->addWidget(m_cboPayee,   0, 5);
    layoutSchedule->addWidget(lblMemo,      1, 0);
    layoutSchedule->addWidget(m_txtMemo,    1, 1, 1, 5);
    layoutSchedule->addWidget(m_splitsWidget, 2, 0, 1, 5);

    //Buttons
    m_btnPrevious = new QPushButton(Core::icon("go-left"), tr("&Previous"), this);
    m_btnNext = new QPushButton(Core::icon("go-right"), tr("Ne&xt"), this);
    m_btnEnter = new QPushButton(Core::icon("key-enter"), tr("&Enter"), this);

    connect(m_btnPrevious, &QPushButton::clicked, this, &ScheduleEntryForm::showPrevious);
    connect(m_btnNext,     &QPushButton::clicked, this, &ScheduleEntryForm::showNext);
    connect(m_btnEnter,    &QPushButton::clicked, this, &ScheduleEntryForm::enterCurrent);

    QHBoxLayout* layoutButtons = new QHBoxLayout();
    layoutButtons->addWidget(m_btnPrevious);
    layoutButtons->addStretch(1);
    layoutButtons->addWidget(m_btnEnter);
    layoutButtons->addStretch(1);
    layoutButtons->addWidget(m_btnNext);

    //Main interface
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(lblInfos);
    mainLayout->addWidget(m_frmSchedule);
    mainLayout->addLayout(layoutButtons);

    //-------------- Load the first schedule ----------------
    loadSchedule(0, false);

}

void ScheduleEntryForm::showPrevious()
{
    loadSchedule(m_currentIdx-1);
}

void ScheduleEntryForm::showNext()
{
    loadSchedule(m_currentIdx+1);
}

void ScheduleEntryForm::enterCurrent()
{
    //Validate
    if (!m_splitsWidget->validate())
    {
        return;
    }

    if (!m_dteDate->date().isValid())
    {
        QMessageBox::information(this,
                                 tr("Enter Transaction"),
                                 tr("The transaction date is invalid."));
        m_dteDate->setFocus();
        return;
    }

    //Payee
    int idPayee = Constants::NO_ID;
    if (!m_cboPayee->currentText().isEmpty())
    {
        try
        {
            idPayee = PayeeManager::instance()->get(m_cboPayee->currentText())->id();
        }
        catch (ModelException)
        {
            int ans = QMessageBox::question(this,
                                            tr("Save Changes"),
                                            tr("The payee you entered does not currently "
                                               "exists. Do you want to add it, have no "
                                               "payee for this transaction, or cancel?"),
                                            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

            switch (ans)
            {
            case QMessageBox::Yes:
                idPayee = PayeeManager::instance()->add(m_cboPayee->currentText())->id();
                break;
            case QMessageBox::No:
                idPayee = -1;
                break;
            default:
                return;
            }
        }
    }

    //We're ready to add it!
    try
    {
//        m_schedules[m_currentIdx]->enterNext(m_txtNo->text(),
//                                             m_dteDate->date(),
//                                             m_txtMemo->text(),
//                                             idPayee,
//                                             m_splitsWidget->validSplits());
    }
    catch (ModelException e)
    {
        QMessageBox::warning(this,
                             tr("Enter Transaction"),
                             tr("An error has occured while adding the transaction:\n\n%1").arg(e.description()));
        return;
    }

    m_schedules.removeAt(m_currentIdx);
    m_transactions.removeAt(m_currentIdx);

    if (m_schedules.count() == 0)                   //In this case, we're done!
    {
        close();
    }
    else if (m_schedules.count() >= m_currentIdx)   //If we're out of bounds...
    {
        loadSchedule(m_currentIdx-1, false);
    }
    else                                            //Otherwise, this index corresponds to another schedule, just reload.
    {
        loadSchedule(m_currentIdx, false);
    }

}

void ScheduleEntryForm::loadSchedule(int _idx, bool _save)
{
    if (_idx < 0 || _idx >= m_transactions.count())
        return;


    //Save current
    if (_save)
    {
        m_transactions[m_currentIdx].memo   = m_txtMemo->text();
        m_transactions[m_currentIdx].payee  = m_cboPayee->currentText();
        m_transactions[m_currentIdx].date   = m_dteDate->date();
        m_transactions[m_currentIdx].no     = m_txtNo->text();
        m_transactions[m_currentIdx].splits = m_splitsWidget->validSplits();
    }

    //Buttons, indices
    m_currentIdx = _idx;
    m_btnPrevious->setEnabled(m_currentIdx > 0);
    m_btnNext->setEnabled(m_currentIdx < m_transactions.count()-2);


    //Load data
    m_txtMemo->setText(m_transactions[m_currentIdx].memo);
    m_cboPayee->setCurrentText(m_transactions[m_currentIdx].memo);
    m_dteDate->setDate(m_transactions[m_currentIdx].date);
    m_txtNo->setText(m_transactions[m_currentIdx].no);
    m_splits = m_transactions[m_currentIdx].splits;
    m_splitsWidget->reload();

}
