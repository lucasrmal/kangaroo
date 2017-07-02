#include "formeditschedule.h"

#include "../core.h"
#include "../../model/schedule.h"
#include "../../model/modelexception.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QComboBox>
#include <QStackedWidget>
#include <QGridLayout>

namespace KLib
{
    FormEditSchedule::FormEditSchedule(Schedule* _schedule, QWidget *parent) :
        CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
        m_schedule(_schedule)
    {
        setupUI();
        fillFrequencies();
        onStoppingChanged();
        onFrequencyChanged();
        loadData();
    }

    void FormEditSchedule::accept()
    {
        // Validate

        QStringList errors;

        if (!m_dteStarts->date().isValid())
        {
            errors << tr("The start date is invalid.");
        }

        if (m_optEndDate->isChecked() && !m_dteEndDate->date().isValid())
        {
            errors << tr("The end date is invalid.");
        }

        if (m_optEndDate->isChecked() && m_dteStarts->date().isValid() && m_dteEndDate->date().isValid()
            && m_dteStarts->date() > m_dteEndDate->date())
        {
            errors << tr("The end date is before the start date.");
        }

        if (m_optNumRemaining->isChecked() && m_spinRemaining->value() == 0)
        {
            errors << tr("The number of remaining instances must be greater than zero.");
        }

        switch (m_cboFrequency->currentData().toInt())
        {
        case Frequency::Weekly:
        {
            int count = 0;

            for (int i = 0; i < 7; ++i)
            {
                if (m_chkDayOfWeek[i]->isChecked())
                {
                    ++count;
                }
            }

            if (count == 0)
            {
                errors << tr("At least one week day must be selected.");
            }
            break;
        }
        }

        if (errors.count() > 0)
        {
            QString strErrors;
            for (QString s : errors)
            {
                strErrors += "\t" + s + "\n";
            }
            QMessageBox::information(this,
                                     tr("Save Changes"),
                                     tr("The following errors prevent the schedule to be "
                                        "saved:\n %1.").arg(strErrors),
                                     QMessageBox::Ok);
            return;
        }

        //Everything is fine, save the schedule

        Recurrence r;
        r.beginDate = m_dteStarts->date();
        r.frequency = m_cboFrequency->currentData().toInt();
        r.every = m_cboFrequency->currentData().toInt() == Frequency::Once ? 1 : m_spinEvery->value();

        if (m_optNotStopping->isChecked())
        {
            r.stops = false;
        }
        else if (m_optEndDate->isChecked())
        {
            r.stops = true;
            r.lastDate = m_dteEndDate->date();
        }
        else //Num remaining
        {
            r.stops = true;
            r.numRemaining = m_spinRemaining->value();
        }

        switch (m_cboFrequency->currentData().toInt())
        {
        case Frequency::Weekly:
            r.weekdays.clear();
            for (int i = 0; i < 7; ++i)
            {
                if (m_chkDayOfWeek[i]->isChecked())
                {
                    r.weekdays.insert(static_cast<Qt::DayOfWeek>(i+1));
                }
            }
            break;

        case Frequency::Monthly:
            r.daysOfMonth.clear();

            for (int i = 0; i < MAX_INSTANCES_DATE; ++i)
            {
                if (m_cboDaysOfMonth[i]->isVisible())
                {
                    r.daysOfMonth << m_cboDaysOfMonth[i]->currentData().toInt();
                }
            }
            break;

        case Frequency::Yearly:
            r.daysOfYear.clear();
            r.daysOfYear << DayMonth(m_cboMonth->currentIndex()+1, m_cboDayOfMonth->currentData().toInt());
            break;
        }

        m_schedule->setDescription(m_txtName->text());

        try
        {
            m_schedule->holdToModify();
            m_schedule->setDescription(m_txtName->text());
            m_schedule->setRemindBefore(m_chkRemind->isChecked() ? m_spinRemindBefore->value()
                                                                 : -1);
            m_schedule->setRecurrence(r);
            m_schedule->doneHoldToModify();
            done(QDialog::Accepted);
        }
        catch (ModelException e)
        {
            QMessageBox::warning(this,
                                 tr("Schedule"),
                                 tr("An error has occured while saving the schedule:\n\n%1").arg(e.description()));
        }
    }

    void FormEditSchedule::onStoppingChanged()
    {
        m_dteEndDate->setEnabled(m_optEndDate->isChecked());
        m_spinRemaining->setEnabled(m_optNumRemaining->isChecked());
    }

    void FormEditSchedule::onFrequencyChanged()
    {
        switch (m_cboFrequency->currentData().toInt())
        {
        case Frequency::Once:
            m_layoutFrequency->setCurrentIndex(Widget_Once);
            m_lblReccurence->setText("");
            m_lblEveryType->setText("");
            break;

        case Frequency::Daily:
            m_layoutFrequency->setCurrentIndex(Widget_Day);
            m_lblReccurence->setText("");
            m_lblEveryType->setText(tr("day(s)"));
            break;

        case Frequency::Weekly:
            m_layoutFrequency->setCurrentIndex(Widget_Week);
            m_lblReccurence->setText(tr("Days:"));
            m_lblEveryType->setText(tr("week(s)"));
            break;

        case Frequency::Monthly:
            m_layoutFrequency->setCurrentIndex(Widget_Month);
            m_lblReccurence->setText(tr("On the:"));
            m_lblEveryType->setText(tr("month(s)"));
            break;

        case Frequency::Yearly:
            m_layoutFrequency->setCurrentIndex(Widget_Year);
            m_lblReccurence->setText(tr("Month, Day:"));
            m_lblEveryType->setText(tr("year(s)"));
            break;

        }

        m_spinEvery->setEnabled(m_cboFrequency->currentData().toInt() != Frequency::Once);
    }

    void FormEditSchedule::onMonthChanged()
    {
        setDaysInMonth(m_cboDayOfMonth, m_cboMonth->currentIndex()+1);
    }
    void FormEditSchedule::loadData()
    {
        m_txtName->setText(m_schedule->description());

        m_chkRemind->setChecked(m_schedule->remindBefore() >= 0);
        if (m_chkRemind->isChecked()) m_spinRemindBefore->setValue(m_schedule->remindBefore());
        onReminderToggled(m_chkRemind->isChecked());

        m_optNotStopping->setChecked(!m_schedule->recurrence().stops);
        m_optEndDate->setChecked(m_schedule->recurrence().lastDate.isValid());
        m_optNumRemaining->setChecked(m_schedule->recurrence().numRemaining >= 0);

        m_cboFrequency->setCurrentIndex(m_cboFrequency->findData(m_schedule->recurrence().frequency));
        m_dteStarts->setDate(m_schedule->recurrence().beginDate);

        if (m_schedule->recurrence().frequency != Frequency::Once)
        {
            m_spinEvery->setValue(m_schedule->recurrence().every);
        }
        else
        {
            m_spinEvery->setValue(1);
        }

        for (int i = 0; i < 7; ++i)
        {
            m_chkDayOfWeek[i]->setChecked(false);
        }

        if (m_schedule->recurrence().frequency == Frequency::Weekly)
        {
            for (Qt::DayOfWeek d : m_schedule->recurrence().weekdays)
            {
                m_chkDayOfWeek[((int) d)-1]->setChecked(true);
            }
        }

        if (m_schedule->recurrence().frequency == Frequency::Monthly)
        {
            showDaysOfMonth(m_schedule->recurrence().daysOfMonth.count());

            int i = 0;
            for (int d : m_schedule->recurrence().daysOfMonth)
            {
                m_cboDaysOfMonth[i]->setCurrentIndex(m_cboDaysOfMonth[i]->findData(d));
                ++i;
            }
        }
        else
        {
            showDaysOfMonth(1);
        }

        if (m_schedule->recurrence().frequency == Frequency::Yearly)
        {
            for (DayMonth d : m_schedule->recurrence().daysOfYear)
            {
                m_cboMonth->setCurrentIndex(d.first-1);
                m_cboDayOfMonth->setCurrentIndex(m_cboDayOfMonth->findData(d.second));
            }
        }
        else
        {
            m_cboDayOfMonth->setCurrentIndex(0);
            m_cboMonth->setCurrentIndex(0);
        }
    }

    void FormEditSchedule::onReminderToggled(bool _enabled)
    {
        m_spinRemindBefore->setEnabled(_enabled);
    }

    void FormEditSchedule::addInstance()
    {
        if (m_daysOfMonthShown < MAX_INSTANCES_DATE -1)
        {
            ++m_daysOfMonthShown;
            m_btnRemoveInstance[0]->setVisible(m_daysOfMonthShown > 1);
            m_btnAddInstance->setVisible(m_daysOfMonthShown < MAX_INSTANCES_DATE);

            //Show the next one
            for (int i = 0; i < MAX_INSTANCES_DATE; ++i)
            {
                if (m_cboDaysOfMonth[i]->isHidden())
                {
                    m_cboDaysOfMonth[i]->setVisible(true);
                    m_btnRemoveInstance[i]->setVisible(true);
                    m_spacer[i]->setVisible(true);
                    break;
                }
            }
        }
    }

    void FormEditSchedule::removeInstance()
    {
        QPushButton* btn = qobject_cast<QPushButton*>(sender());
        int i = 0;

        for (; i < MAX_INSTANCES_DATE; ++i)
        {
            if (btn == m_btnRemoveInstance[i])
            {
                hideDayOfMonth(i);
                break;
            }
        }
    }

    void FormEditSchedule::showDaysOfMonth(int _numToShow)
    {
        if (_numToShow >= MAX_INSTANCES_DATE
            || _numToShow < 1)
        {
            return;
        }

        m_btnRemoveInstance[0]->setVisible(_numToShow > 1);

        for (int i = 1; i < MAX_INSTANCES_DATE; ++i)
        {
            m_cboDaysOfMonth[i]->setVisible(i < _numToShow);
            m_btnRemoveInstance[i]->setVisible(i < _numToShow);
            m_spacer[i]->setVisible(i < _numToShow);

            if (i >= _numToShow)
            {
                m_cboDaysOfMonth[i]->setCurrentIndex(m_cboDaysOfMonth[i]->findData(1));
            }
        }

        m_daysOfMonthShown = _numToShow;
        m_btnAddInstance->setVisible(m_daysOfMonthShown < MAX_INSTANCES_DATE);
    }

    void FormEditSchedule::hideDayOfMonth(int _index)
    {
        if (m_daysOfMonthShown > 1 && _index < MAX_INSTANCES_DATE)
        {
            m_cboDaysOfMonth[_index]->setCurrentIndex(0);
            m_cboDaysOfMonth[_index]->setVisible(false);
            m_btnRemoveInstance[_index]->setVisible(false);
            m_spacer[_index]->setVisible(false);

            --m_daysOfMonthShown;

            if (m_daysOfMonthShown == 1)
            {
                for (int j; j < MAX_INSTANCES_DATE; ++j)
                {
                    if (m_cboDaysOfMonth[j]->isVisible())
                    {
                        m_btnRemoveInstance[j]->setVisible(false);
                    }
                }
            }
        }
    }

    void FormEditSchedule::fillFrequencies()
    {
        m_cboFrequency->clear();

        m_cboFrequency->addItem(Frequency::frequencyToString(Frequency::Once),    Frequency::Once);
        m_cboFrequency->addItem(Frequency::frequencyToString(Frequency::Daily),   Frequency::Daily);
        m_cboFrequency->addItem(Frequency::frequencyToString(Frequency::Weekly),  Frequency::Weekly);
        m_cboFrequency->addItem(Frequency::frequencyToString(Frequency::Monthly), Frequency::Monthly);
        m_cboFrequency->addItem(Frequency::frequencyToString(Frequency::Yearly),  Frequency::Yearly);
    }

    void FormEditSchedule::setupUI()
    {
        setBothTitles(tr("Edit Schedule"));
        setPicture(Core::pixmap("schedule"));

        QGridLayout* mainLayout = new QGridLayout(centralWidget());

        m_txtName = new QLineEdit(this);
        QLabel* lblName = new QLabel(tr("&Name:"), this);
        lblName->setBuddy(m_txtName);

        m_chkRemind = new QCheckBox(tr("Show reminder "), this);
        m_spinRemindBefore = new QSpinBox(this);

        m_spinRemindBefore->setMinimum(0);
        m_spinRemindBefore->setMaximum(30);
        m_spinRemindBefore->setValue(5);

        mainLayout->addWidget(lblName, 0, 0);
        mainLayout->addWidget(m_txtName, 0, 1);

        QHBoxLayout* layoutReminder = new QHBoxLayout();
        layoutReminder->addWidget(m_chkRemind);
        layoutReminder->addWidget(m_spinRemindBefore);
        layoutReminder->addWidget(new QLabel(tr(" days before the due date."), this));

        QFrame* line1 = new QFrame(this);
        line1->setFrameShape(QFrame::HLine);

        mainLayout->addLayout(layoutReminder, 2, 1);
        mainLayout->addWidget(line1, 3, 1);

        //------------------------- Reccurence ---------------------------

        m_dteStarts = new QDateEdit(this);

        m_optNotStopping = new QRadioButton(tr("Ne&ver"), this);

        m_optEndDate = new QRadioButton(tr("&Until "), this);
        m_dteEndDate = new QDateEdit(this);
        m_optNumRemaining = new QRadioButton(tr("A&fter "), this);
        m_spinRemaining = new QSpinBox(this);

        m_cboFrequency = new QComboBox(this);
        m_spinEvery = new QSpinBox(this);

        QLabel* lblStops = new QLabel(tr("Stops:"), this);
        QLabel* lblStarts = new QLabel(tr("St&arts:"), this);
        QLabel* lblFreq = new QLabel(tr("&Frequency:"), this);
        QLabel* lblEvery = new QLabel(tr("Ever&y:"), this);
        m_lblEveryType = new QLabel(this);
        lblFreq->setBuddy(m_cboFrequency);
        lblStarts->setBuddy(m_dteStarts);
        lblEvery->setBuddy(m_spinEvery);

        m_lblReccurence = new QLabel(this);

        //Monthly, yearly
        for (int i = 0; i < MAX_INSTANCES_DATE; ++i)
        {
            m_btnRemoveInstance[i] = new QPushButton(Core::icon("list-remove"), "", this);
            m_btnRemoveInstance[i]->setMaximumWidth(24);

            connect(m_btnRemoveInstance[i], SIGNAL(clicked()), this, SLOT(removeInstance()));
        }

        m_btnAddInstance = new QPushButton(Core::icon("list-add"), "", this);
        m_btnAddInstance->setMaximumWidth(24);

        //Monthly, bimonthly
        for (int i = 0; i < MAX_INSTANCES_DATE; ++i)
        {
            m_cboDaysOfMonth[i] = new QComboBox(this);
            m_spacer[i] = new QWidget();
            m_spacer[i]->setFixedWidth(10);
            setDaysInMonth(m_cboDaysOfMonth[i], 1);
            m_cboDaysOfMonth[i]->setCurrentIndex(m_cboDaysOfMonth[i]->findData(1));
        }

        //Yearly
        m_cboMonth = new QComboBox(this);
        m_cboDayOfMonth = new QComboBox(this);

        m_layoutFrequency = new QStackedWidget();
        QWidget* widgetFreq[NUM_WIDGETS];
        QHBoxLayout* layoutFreq[NUM_WIDGETS];

        m_layoutFrequency->setContentsMargins(0, 0, 0, 0);
        m_layoutFrequency->setMaximumHeight(m_cboMonth->sizeHint().height()+5);

        for (int i = 0; i < NUM_WIDGETS; ++i)
        {
            widgetFreq[i] = new QWidget();
            layoutFreq[i] = new QHBoxLayout(widgetFreq[i]);
            layoutFreq[i]->setContentsMargins(0, 0, 0, 0);
            m_layoutFrequency->addWidget(widgetFreq[i]);
        }

        //Weekly
        for (int i = 0; i < 7; ++i)
        {
            m_chkDayOfWeek[i] = new QCheckBox(QDate::shortDayName(i+1, QDate::StandaloneFormat), this);
            layoutFreq[Widget_Week]->addWidget(m_chkDayOfWeek[i]);
        }
        layoutFreq[Widget_Week]->addStretch(1);

        //Monthly
        for (int i = 0; i < MAX_INSTANCES_DATE; ++i)
        {
            if (i > 0)
            {
                m_cboDaysOfMonth[i]->setVisible(false);
                m_spacer[i]->setVisible(false);
            }

            layoutFreq[Widget_Month]->addWidget(m_cboDaysOfMonth[i]);
            layoutFreq[Widget_Month]->addWidget(m_btnRemoveInstance[i]);
            layoutFreq[Widget_Month]->addWidget(m_spacer[i]);
            //layoutFreq[Widget_Month]->addSpacing(10);
            m_btnRemoveInstance[i]->setVisible(false);
        }
        layoutFreq[Widget_Month]->addWidget(m_btnAddInstance);
        layoutFreq[Widget_Month]->addStretch(1);

        //Yearly
        layoutFreq[Widget_Year]->addWidget(m_cboMonth);
        layoutFreq[Widget_Year]->addWidget(m_cboDayOfMonth);
        layoutFreq[Widget_Year]->addStretch(1);

        QHBoxLayout* layoutStops = new QHBoxLayout();
        layoutStops->addWidget(m_optNotStopping);
        layoutStops->addWidget(m_optEndDate);
        layoutStops->addWidget(m_dteEndDate);
        layoutStops->addWidget(m_optNumRemaining);
        layoutStops->addWidget(m_spinRemaining);
        layoutStops->addWidget(m_cboFrequency);
        layoutStops->addWidget(new QLabel("instances"));
        layoutStops->addStretch(2);

        mainLayout->addWidget(lblStarts,         4, 0);
        mainLayout->addWidget(m_dteStarts,       4, 1);
        mainLayout->addWidget(lblStops,          5, 0);
        mainLayout->addLayout(layoutStops,       5, 1, 1, 3);


        QFrame* line2 = new QFrame(this);
        line2->setFrameShape(QFrame::HLine);
        mainLayout->addWidget(line2, 6, 1, 1, 3);


        mainLayout->addWidget(lblFreq,           7, 0);
        mainLayout->addWidget(m_cboFrequency,    7, 1);
        mainLayout->addWidget(lblEvery,          7, 2);
        mainLayout->addWidget(m_spinEvery,       7, 3);
        mainLayout->addWidget(m_lblEveryType,    7, 4);
        mainLayout->addWidget(m_lblReccurence,   8, 0);
        mainLayout->addWidget(m_layoutFrequency, 8, 1, 1, 4);
        mainLayout->setColumnMinimumWidth(0, 90);
        mainLayout->setColumnStretch(1, 1);
        mainLayout->setColumnStretch(4, 2);

        m_optNotStopping->setChecked(true);

        for (int i = 0; i < 12; ++i)
        {
            m_cboMonth->addItem(QDate::longMonthName(i+1, QDate::StandaloneFormat));
        }

        setDaysInMonth(m_cboDayOfMonth, 1);

        m_spinEvery->setMinimum(1);
        m_spinEvery->setMaximum(365);

        m_spinRemaining->setMinimum(0);
        m_spinRemaining->setMaximum(10000);

        Core::setupDateEdit(m_dteEndDate);
        Core::setupDateEdit(m_dteStarts);


        connect(m_btnAddInstance, SIGNAL(clicked()), this, SLOT(addInstance()));
        connect(m_optEndDate, SIGNAL(toggled(bool)), this, SLOT(onStoppingChanged()));
        connect(m_optNotStopping, SIGNAL(toggled(bool)), this, SLOT(onStoppingChanged()));
        connect(m_optNumRemaining, SIGNAL(toggled(bool)), this, SLOT(onStoppingChanged()));
        connect(m_cboFrequency, SIGNAL(currentIndexChanged(int)), this, SLOT(onFrequencyChanged()));
        connect(m_cboMonth, SIGNAL(currentIndexChanged(int)), this, SLOT(onMonthChanged()));

        connect(m_chkRemind, &QCheckBox::toggled, this, &FormEditSchedule::onReminderToggled);
    }

    void FormEditSchedule::setDaysInMonth(QComboBox* _cbo, int _month)
    {
        int currentIndex = _cbo->currentIndex();
        _cbo->clear();

        //Feb, we don't want to use Feb 29 as it's only every 4 years...
        int numDays = _month == 2 ? 28 : QDate(2000, _month, 1).daysInMonth();

        _cbo->addItem(tr("First weekday"), Recurrence::FIRST_WEEKDAY);
        _cbo->addItem(tr("Last weekday"), Recurrence::LAST_WEEKDAY);
        _cbo->addItem(tr("Last day"), Recurrence::LAST_DAY);

        for (int i = 1; i <= numDays; ++i)
        {
            QString number = QString::number(i);

            if (number.endsWith("1"))
            {
                number = tr("%1st").arg(number);
            }
            else if (number.endsWith("1"))
            {
                number = tr("%1nd").arg(number);
            }
            else if (number.endsWith("1"))
            {
                number = tr("%1rd").arg(number);
            }
            else
            {
                number = tr("%1th").arg(number);
            }

            _cbo->addItem(number, i);
        }

        if (currentIndex >= _cbo->count())
        {
            currentIndex = _cbo->count()-1;
        }
        else if (currentIndex == -1)
        {
            currentIndex = 0;
        }

        _cbo->setCurrentIndex(currentIndex);
    }

}

