#include "formcalculator.h"

#include <KangarooLib/ui/core.h>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <KangarooLib/ui/widgets/amountedit.h>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeyEvent>

using namespace KLib;

const int FormCalculator::PRECISION = 2;

FormCalculator::FormCalculator(QWidget *parent) :
    CAMSEGDialog(DialogWithPicture, CloseButton, parent)
{
    setBothTitles(tr("Investment Calculator"));
    setPicture(Core::pixmap("investment-calculator"));

    m_txtInitial = new AmountEdit(PRECISION, this);
    m_txtRecurring = new AmountEdit(PRECISION, this);
    m_cboRecurring = new QComboBox(this);

    m_spinPeriods = new QSpinBox(this);
    m_cboPeriods = new QComboBox(this);

    m_txtInterest = new AmountEdit(3, this);
    m_cboCompounded = new QComboBox(this);

    m_lblPrincipal = new QLabel(this);
    m_lblInterest = new QLabel(this);
    m_lblTotal = new QLabel(this);

    m_spinPeriods->setMaximum(100000);
    m_spinPeriods->setMinimum(0);

    QPushButton* btnCalculate = new QPushButton(Core::icon("execute"), tr("Calculate"), this);
    connect(btnCalculate, SIGNAL(clicked()), this, SLOT(calculate()));

    QFormLayout* layout = new QFormLayout(centralWidget());
    layout->addRow(tr("&Initial Investment:"), m_txtInitial);

    QHBoxLayout* layoutRecurring = new QHBoxLayout();
    QLabel* lblRec = new QLabel(tr("&Recurring Investment:"));
    lblRec->setBuddy(m_txtRecurring);
    layoutRecurring->addWidget(m_txtRecurring);
    layoutRecurring->addWidget(m_cboRecurring);
    layout->addRow(lblRec, layoutRecurring);

    QHBoxLayout* layoutPeriod = new QHBoxLayout();
    QLabel* lblPer = new QLabel(tr("&After:"));
    lblPer->setBuddy(m_txtRecurring);
    layoutPeriod->addWidget(m_spinPeriods);
    layoutPeriod->addWidget(m_cboPeriods);
    layout->addRow(lblPer, layoutPeriod);

    layout->addRow(tr("I&nterest per year:"), m_txtInterest);
    layout->addRow(tr("&Compounded:"), m_cboCompounded);

    layout->addWidget(btnCalculate);

    layout->addRow(tr("Principal:"), m_lblPrincipal);
    layout->addRow(tr("Interest:"), m_lblInterest);
    layout->addRow(tr("Total:"), m_lblTotal);

    QStringList periodTypes;
    periodTypes << tr("Per year") << tr("Per month") << tr("Per day");
    m_cboRecurring->addItems(periodTypes);

    QStringList duration;
    duration << tr("Years") << tr("Months") << tr("Days");
    m_cboPeriods->addItems(duration);

    QStringList compoundTypes;
    compoundTypes << tr("Yearly") << tr("Monthy") << tr("Daily");
    m_cboCompounded->addItems(compoundTypes);
}

void FormCalculator::keyPressEvent(QKeyEvent* _event)
{
    if (_event->key() == Qt::Key_Enter
        || _event->key() == Qt::Key_Return)
    {
        if (m_btnClose->hasFocus())
        {
            m_btnClose->click();
        }
        else
        {
            calculate();
        }
    }
    else
    {
        CAMSEGDialog::keyPressEvent(_event);
    }

}

double multiplier(int from, int to)
{
    if (from == to)
    {
        return 1.0;
    }
    else
    {
        switch (to)
        {
        case 0: //Year
            if (from == 1) //Month to Year
            {
                return 12.;
            }
            else //Day to year
            {
                return 365.;
            }
        case 1: //Month
            if (from == 0) //Year to month
            {
                return 1./12.;
            }
            else //Day to month
            {
                return 30.;
            }
        case 2: //Day
            if (from == 0) //Year to day
            {
                return 1./365.;
            }
            else //Month to day
            {
                return 1./30.;
            }
        }
    }

    return 0.;
}

void FormCalculator::calculate()
{
    //Amount interest;

    //int numPeriods;

    //Put everything in the compounded frequency

    Amount rec = m_txtRecurring->amount() * multiplier(m_cboRecurring->currentIndex(), m_cboCompounded->currentIndex());
    double interest = m_txtInterest->amount().toDouble()
                      * multiplier(Year, m_cboCompounded->currentIndex())
                      /100.0;
    double numPeriods = floor(((double)m_spinPeriods->value()) * multiplier(m_cboCompounded->currentIndex(), m_cboPeriods->currentIndex()));

    Amount total = 0;
    Amount principal = m_txtInitial->amount() + rec * numPeriods;

    if (interest == 0)
    {
        total = principal;
    }
    else
    {
        total = rec* ((pow(1.0+interest,numPeriods+1.0) - (1.0+interest)) / interest)
                + m_txtInitial->amount() * pow(1.0+interest, numPeriods);
                //D( ( (1 + r/n)nt – 1) / (r/n) ) + B(1 + r/n)nt
    }

    m_lblPrincipal->setText(principal.toString());
    m_lblInterest->setText((total-principal).toString());
    m_lblTotal->setText(total.toString());




}
