#include "dateintervalselector.h"

#include <QComboBox>
#include <QSpinBox>
#include <QHBoxLayout>

namespace KLib
{

    DateIntervalSelector::DateIntervalSelector(int _flags, QWidget *parent) :
        QWidget(parent),
        m_flags(_flags)
    {
        m_cboBalanceSelector = new QComboBox(this);
        m_spinYear = new QSpinBox(this);
        m_cboMonth = new QComboBox(this);

        m_spinYear->setMinimum(1900);
        m_spinYear->setMaximum(9999);

        for (int i = 1; i <= 12; ++i)
            m_cboMonth->addItem(QDate::longMonthName(i));

        m_cboMonth->setCurrentIndex(QDate::currentDate().month()-1);

        if (m_flags | Flag_ThroughToday)
        {
            m_cboBalanceSelector->addItem(tr("Through Today"), Flag_ThroughToday);
        }

        if (m_flags | Flag_AllTime)
        {
            m_cboBalanceSelector->addItem(tr("All Time"), Flag_AllTime);
        }

        if (m_flags | Flag_YTD)
        {
            m_cboBalanceSelector->addItem(tr("YTD"), Flag_YTD);
        }

        if (m_flags | Flag_Year)
        {
            m_cboBalanceSelector->addItem(tr("Year"), Flag_Year);
        }

        if (m_flags | Flag_Month)
        {
            m_cboBalanceSelector->addItem(tr("Month"), Flag_Month);
        }

        m_cboBalanceSelector->setMaximumWidth(200);

        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->addWidget(m_cboBalanceSelector);
        layout->addWidget(m_spinYear);
        layout->addWidget(m_cboMonth);

        layout->setContentsMargins(0,0,0,0);

        //Set default dates
        m_spinYear->setValue(QDate::currentDate().year());
        m_cboMonth->setCurrentIndex(QDate::currentDate().month()-1);
        onChangeBalanceType(m_cboBalanceSelector->currentIndex());

        connect(m_cboBalanceSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeBalanceType(int)));
        connect(m_spinYear, SIGNAL(valueChanged(int)), this, SLOT(onMonthOrYearChanged()));
        connect(m_cboMonth, SIGNAL(currentIndexChanged(int)), this, SLOT(onMonthOrYearChanged()));
    }

    void DateIntervalSelector::setYear(int _year)
    {
        if (m_flags | Flag_Year)
        {
            m_cboBalanceSelector->setCurrentIndex(m_cboBalanceSelector->findData(Flag_Year));
            m_spinYear->setValue(_year);
        }
    }

    void DateIntervalSelector::setMonth(int _month, int _year)
    {
        if (m_flags | Flag_Month)
        {
            m_cboBalanceSelector->setCurrentIndex(m_cboBalanceSelector->findData(Flag_Month));
            m_spinYear->setValue(_year);
            m_cboMonth->setCurrentIndex(_month-1);
        }
    }

    void DateIntervalSelector::setAllTime()
    {
        if (m_flags | Flag_AllTime)
        {
            m_cboBalanceSelector->setCurrentIndex(m_cboBalanceSelector->findData(Flag_AllTime));
        }
    }

    void DateIntervalSelector::setThroughToday()
    {
        if (m_flags | Flag_ThroughToday)
        {
            m_cboBalanceSelector->setCurrentIndex(m_cboBalanceSelector->findData(Flag_ThroughToday));
        }
    }

    void DateIntervalSelector::setYTD()
    {
        if (m_flags | Flag_YTD)
        {
            m_cboBalanceSelector->setCurrentIndex(m_cboBalanceSelector->findData(Flag_YTD));
        }
    }

    void DateIntervalSelector::onMonthOrYearChanged()
    {
        if (m_cboBalanceSelector->currentIndex() == -1)
            return;

        emit intervalChanged(interval());
    }

    void DateIntervalSelector::onChangeBalanceType(int _index)
    {
        if (_index == -1)
            return;

        switch (m_cboBalanceSelector->currentData().toInt())
        {
        case Flag_ThroughToday:
        case Flag_AllTime:
        case Flag_YTD:
            m_cboMonth->setEnabled(false);
            m_spinYear->setEnabled(false);
            break;

        case Flag_Year:
            m_cboMonth->setEnabled(false);
            m_spinYear->setEnabled(true);
            m_spinYear->setFocus();
            break;

        case Flag_Month:
            m_cboMonth->setEnabled(true);
            m_spinYear->setEnabled(true);
            m_cboMonth->setFocus();
            break;
        }

        emit intervalChanged(interval());
    }

    DateInterval DateIntervalSelector::interval() const
    {
        QDate begin, end;

        if (m_cboBalanceSelector->currentIndex() == -1)
            return DateInterval(begin, end);

        switch (m_cboBalanceSelector->currentData().toInt())
        {
        case Flag_ThroughToday:
            end = QDate::currentDate();
            break;

        case Flag_YTD:
            end = QDate::currentDate();
            begin = QDate(end.year(), 1, 1);
            break;

        case Flag_Year:
            begin = QDate(m_spinYear->value(), 1, 1);
            end = QDate(m_spinYear->value(), 12, 31);
            break;

        case Flag_Month:
            begin = QDate(m_spinYear->value(), m_cboMonth->currentIndex()+1, 1);
            end = QDate(m_spinYear->value(), m_cboMonth->currentIndex()+1, begin.daysInMonth());
            break;

        case Flag_AllTime: //Use invalid dates, so nothing to do
            break;
        }

        return DateInterval(begin, end);
    }

}

