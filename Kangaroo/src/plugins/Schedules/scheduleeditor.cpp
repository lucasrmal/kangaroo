#include "scheduleeditor.h"

#include <QTreeView>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <KangarooLib/ui/widgets/splitswidget.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/controller/payeecontroller.h>
#include <KangarooLib/model/payee.h>
#include <KangarooLib/model/transaction.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/model/schedule.h>
#include <KangarooLib/controller/schedulecontroller.h>
#include <QSortFilterProxyModel>

using namespace KLib;

//const int ScheduleEditor::MAX_INSTANCES_DATE = 5;

ScheduleEditor::ScheduleEditor(QWidget *parent) :
    QWidget(parent),
    m_daysOfMonthShown(1)
{
    setupUI();
    fillFrequencies();
    onStoppingChanged();
    onFrequencyChanged();
    clear();
}

void ScheduleEditor::onStoppingChanged()
{
    m_dteEndDate->setEnabled(m_optEndDate->isChecked());
    m_spinRemaining->setEnabled(m_optNumRemaining->isChecked());
}

void ScheduleEditor::onFrequencyChanged()
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

void ScheduleEditor::onMonthChanged()
{
    setDaysInMonth(m_cboDayOfMonth, m_cboMonth->currentIndex()+1);
}

void ScheduleEditor::updateImbalances()
{
    m_lblImbalances->setText(tr("Imbalances:%1").arg(Transaction::splitsImbalances(m_splitEditor->validSplits())));
}

void ScheduleEditor::addNew()
{
    if (m_frmSchedule->isEnabled() && !saveChanges())
    {
        return;
    }

    clear();
    m_frmSchedule->setEnabled(true);

    m_txtName->setFocus();
    m_frmSchedule->setTitle(tr("New Schedule"));
}

void ScheduleEditor::loadSchedule(const QModelIndex& _index)
{
    if (!_index.internalPointer()) //Folder
    {
        m_btnRemove->setEnabled(false);
        return;
    }

    m_btnRemove->setEnabled(true);

    if (m_frmSchedule->isEnabled() && !saveChanges(true))
    {
        return;
    }

    m_current = static_cast<Schedule*>(_index.internalPointer());
    reloadSchedule();
}

bool ScheduleEditor::saveChanges(bool _ask)
{
    if (_ask)
    {
        int ans = QMessageBox::question(this,
                                        tr("Save Changes"),
                                        tr("Do you want to save the changes, discard them or cancel?"),
                                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                        QMessageBox::Save);

        if (ans == QMessageBox::Discard)
        {
            return true;
        }
        else if (ans == QMessageBox::Cancel)
        {
            return false;
        }
    }

    // Validate
    if (!m_splitEditor->validate())
    {
        return false;
    }

    QStringList errors;

    if (m_txtName->text().isEmpty())
    {
        errors << tr("The schedule name is empty.");
    }

    if (!m_dteStarts->date().isValid())
    {
        errors << tr("The start date is invalid.");
    }
//    else if (m_current && m_current->upToDate().isValid() && m_current->upToDate() > m_dteStarts->date())
//    {
//        errors << tr("The start date must be at least %1, as all "
//                     "previous instances of the schedules were entered "
//                     "until this point.").arg(m_current->upToDate().toString(Qt::TextDate));
//    }

    if (m_optEndDate->isChecked()
        && (!m_dteEndDate->date().isValid() || (m_isActive->isChecked() && m_dteEndDate->date() <= QDate::currentDate())))
    {
        errors << tr("The end date is invalid.");
    }

    if (m_optEndDate->isChecked() && m_dteStarts->date().isValid() && m_dteEndDate->date().isValid()
        && m_dteStarts->date() > m_dteEndDate->date())
    {
        errors << tr("The end date is before the start date.");
    }

    if (m_optNumRemaining->isChecked() && m_isActive->isChecked() && m_spinRemaining->value() == 0)
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
        return false;
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
                return false;
            }
        }
    }

    //Everything is fine, add or save the schedule


    Recurrence r;
    r.beginDate = m_dteStarts->date();
    r.frequency = m_cboFrequency->currentData().toInt();
    r.every = m_cboFrequency->currentData().toInt() == Frequency::Once ? 1 : m_spinEvery->value();

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

    Transaction* trans = new Transaction();
    trans->setMemo(m_txtMemo->text());
    trans->setIdPayee(idPayee);
    trans->setSplits(m_splits);

    try
        {

        if (m_current)
        {
            m_current->holdToModify();
            m_current->setDescription(m_txtName->text());
            m_current->setActive(m_isActive->isChecked());
            m_current->setAutoEnter(m_autoEnter->isChecked());
            m_current->setRecurrence(r);
            m_current->setTransaction(trans);
            m_current->doneHoldToModify();

            clear();
        }
        else
        {
            Schedule* sc = ScheduleManager::instance()->add(m_txtName->text(),
                                                            m_autoEnter,
                                                            r,
                                                            trans);
            sc->setActive(m_isActive->isChecked());

            clear();
        }
    }
    catch (ModelException e)
    {
        QMessageBox::warning(this,
                             tr("Schedule"),
                             tr("An error has occured while adding/removing the schedule:\n\n%1").arg(e.description()));

        if (m_current)
        {
            m_current->doneHoldToModify();
        }
        return false;
    }

    return true;
}

void ScheduleEditor::cancel()
{
    int ans = QMessageBox::question(this,
                                    tr("Cancel"),
                                    tr("Are you sure you want to cancel?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);

    if (ans == QMessageBox::No)
    {
        return;
    }

    clear();
}

void ScheduleEditor::reloadSchedule()
{
    m_frmSchedule->setTitle(tr("Edit %1").arg(m_current->description()));

    m_txtName->setText(m_current->description());
    m_isActive->setChecked(m_current->isActive());
    m_autoEnter->setChecked(m_current->autoEnter());

    m_optNotStopping->setChecked(!m_current->recurrence().stops);
    m_optEndDate->setChecked(m_current->recurrence().lastDate.isValid());
    m_optNumRemaining->setChecked(m_current->recurrence().numRemaining >= 0);

    m_cboFrequency->setCurrentIndex(m_cboFrequency->findData(m_current->recurrence().frequency));
    m_dteStarts->setDate(m_current->recurrence().beginDate);

    if (m_current->recurrence().frequency != Frequency::Once)
    {
        m_spinEvery->setValue(m_current->recurrence().every);
    }
    else
    {
        m_spinEvery->setValue(1);
    }

    for (int i = 0; i < 7; ++i)
    {
        m_chkDayOfWeek[i]->setChecked(false);
    }

    if (m_current->recurrence().frequency == Frequency::Weekly)
    {
        for (Qt::DayOfWeek d : m_current->recurrence().weekdays)
        {
            m_chkDayOfWeek[((int) d)-1]->setChecked(true);
        }
    }

    if (m_current->recurrence().frequency == Frequency::Monthly)
    {
        showDaysOfMonth(m_current->recurrence().daysOfMonth.count());

        int i = 0;
        for (int d : m_current->recurrence().daysOfMonth)
        {
            m_cboDaysOfMonth[i]->setCurrentIndex(m_cboDaysOfMonth[i]->findData(d));
            ++i;
        }
    }
    else
    {
        showDaysOfMonth(1);
    }

    if (m_current->recurrence().frequency == Frequency::Yearly)
    {
        for (DayMonth d : m_current->recurrence().daysOfYear)
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

    m_txtMemo->setText(m_current->transaction()->memo());
    m_cboPayee->setCurrentIndex(m_cboPayee->findData(m_current->transaction()->idPayee()));

    m_splits.clear();
    m_splits.append(m_current->transaction()->splits());
    m_splitEditor->reload();

    m_frmSchedule->setEnabled(true);

    updateImbalances();
}

void ScheduleEditor::clear()
{
    m_frmSchedule->setTitle("");

    m_txtName->clear();
    m_isActive->setChecked(true);
    m_autoEnter->setChecked(true);
    m_optNotStopping->setChecked(true);
    m_dteEndDate->setDate(QDate::currentDate().addYears(1));
    m_spinRemaining->setValue(1);

    m_cboFrequency->setCurrentIndex(0);
    m_dteStarts->setDate(QDate::currentDate().addDays(1));
    m_spinEvery->setValue(1);

    for (int i = 0; i < 7; ++i)
    {
        m_chkDayOfWeek[i]->setChecked(false);
    }

    showDaysOfMonth(1);
    m_cboDayOfMonth->setCurrentIndex(0);
    m_cboMonth->setCurrentIndex(0);

    m_txtMemo->clear();
    m_cboPayee->setCurrentText("");
    m_splits.clear();
    m_splitEditor->reload();

    m_frmSchedule->setEnabled(false);
    m_current = NULL;

    updateImbalances();
}

void ScheduleEditor::addInstance()
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

void ScheduleEditor::removeInstance()
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

void ScheduleEditor::showDaysOfMonth(int _numToShow)
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

void ScheduleEditor::hideDayOfMonth(int _index)
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

void ScheduleEditor::fillFrequencies()
{
    m_cboFrequency->clear();

    m_cboFrequency->addItem(Frequency::frequencyToString(Frequency::Once), Frequency::Once);
    m_cboFrequency->addItem(Frequency::frequencyToString(Frequency::Daily), Frequency::Daily);
    m_cboFrequency->addItem(Frequency::frequencyToString(Frequency::Weekly), Frequency::Weekly);
    m_cboFrequency->addItem(Frequency::frequencyToString(Frequency::Monthly), Frequency::Monthly);
    m_cboFrequency->addItem(Frequency::frequencyToString(Frequency::Yearly), Frequency::Yearly);
}

void ScheduleEditor::setupUI()
{

    //-------------------------- Generic frame ----------------------------

    m_txtName = new QLineEdit(this);
    m_isActive = new QCheckBox(tr("Is &Active"), this);
    m_autoEnter = new QCheckBox(tr("Auto &Enter"), this);
    QLabel* lblName = new QLabel(tr("&Name:"), this);
    lblName->setBuddy(m_txtName);

    QHBoxLayout* layoutGeneric = new QHBoxLayout();
    layoutGeneric->addWidget(lblName);
    layoutGeneric->addWidget(m_txtName);
    layoutGeneric->addWidget(m_isActive);
    layoutGeneric->addWidget(m_autoEnter);

    //------------------------- Reccurence frame ---------------------------

    m_frmRecurrence = new QGroupBox(tr("Reccurence"), this);

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

    QGridLayout* layoutRec = new QGridLayout(m_frmRecurrence);
    layoutRec->addWidget(lblStarts,         0, 0);
    layoutRec->addWidget(m_dteStarts,       0, 1);
    layoutRec->addWidget(lblStops,          1, 0);
    layoutRec->addLayout(layoutStops,       1, 1, 1, 3);
    layoutRec->addWidget(lblFreq,           2, 0);
    layoutRec->addWidget(m_cboFrequency,    2, 1);
    layoutRec->addWidget(lblEvery,          2, 2);
    layoutRec->addWidget(m_spinEvery,       2, 3);
    layoutRec->addWidget(m_lblEveryType,    2, 4);
    layoutRec->addWidget(m_lblReccurence,   3, 0);
    layoutRec->addWidget(m_layoutFrequency, 3, 1, 1, 3);
    layoutRec->setColumnMinimumWidth(0, 90);
    layoutRec->setColumnStretch(1, 1);
    layoutRec->setColumnStretch(4, 2);

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

    //------------------------ Transaction frame ---------------------------

    m_frmTransaction = new QGroupBox(tr("Transaction"), this);
    m_txtMemo = new QLineEdit(this);
    m_cboPayee = new QComboBox(this);
    m_splitEditor = new SplitsWidget(m_splits, false, this);
    m_lblImbalances = new QLabel(this);
    QLabel* lblMemo = new QLabel(tr("&Memo:"), this);
    QLabel* lblPayee = new QLabel(tr("&Payee:"), this);
    lblMemo->setBuddy(m_txtMemo);
    lblPayee->setBuddy(lblPayee);

    QGridLayout* layoutTrans = new QGridLayout(m_frmTransaction);
    layoutTrans->addWidget(lblMemo,       0, 0);
    layoutTrans->addWidget(m_txtMemo,     0, 1);
    layoutTrans->addWidget(lblPayee,      0, 2);
    layoutTrans->addWidget(m_cboPayee,    0, 3);
    layoutTrans->addWidget(m_splitEditor, 1, 0, 1, 4);
    layoutTrans->addWidget(m_lblImbalances, 2, 0, 1, 4);

    layoutTrans->setColumnMinimumWidth(3, 180);

    m_cboPayee->setEditable(true);
    m_cboPayee->setModel(PayeeController::sortProxy(this));


    connect(m_splitEditor, SIGNAL(amountChanged()),
            this, SLOT(updateImbalances()));

    //--------------------------- Main Layout ------------------------------

    QGridLayout* mainLayout = new QGridLayout(this);
    m_scheduleView = new QTreeView(this);
    m_frmSchedule = new QGroupBox(tr("Schedule"), this);

    m_btnSave = new QPushButton(Core::icon("save"), tr("&Save Changes"), this);
    m_btnCancel = new QPushButton(Core::icon("dialog-cancel"), tr("Cancel"), this);
    m_btnAdd = new QPushButton(Core::icon("add"), tr("A&dd"), this);
    m_btnRemove = new QPushButton(Core::icon("remove"), tr("Remove"), this);

    m_btnRemove->setEnabled(false);

    QHBoxLayout* layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch(2);
    layoutButtons->addWidget(m_btnCancel);
    layoutButtons->addWidget(m_btnSave);

    QVBoxLayout* layoutRight = new QVBoxLayout(m_frmSchedule);
    layoutRight->addLayout(layoutGeneric);
    layoutRight->addWidget(m_frmRecurrence);
    layoutRight->addWidget(m_frmTransaction);
    layoutRight->addStretch(2);
    layoutRight->addLayout(layoutButtons);

    m_frmRecurrence->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    m_frmTransaction->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    mainLayout->addWidget(m_scheduleView, 0, 0, 1, 2);
    mainLayout->addWidget(m_btnAdd, 1, 0);
    mainLayout->addWidget(m_btnRemove, 1, 1);
    mainLayout->addWidget(m_frmSchedule,  0, 2, 2, 1);
    mainLayout->setColumnStretch(2, 1);

    m_scheduleView->setModel(new ScheduleController(this));
    m_scheduleView->expandAll();
    m_scheduleView->setItemsExpandable(false);

    connect(m_btnCancel, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(m_btnSave, SIGNAL(clicked()), this, SLOT(saveChanges()));
    connect(m_btnAdd, SIGNAL(clicked()), this, SLOT(addNew()));
    connect(m_scheduleView, SIGNAL(entered(QModelIndex)), this, SLOT(loadSchedule(QModelIndex)));
    connect(m_scheduleView, SIGNAL(clicked(QModelIndex)), this, SLOT(loadSchedule(QModelIndex)));
}

void ScheduleEditor::setDaysInMonth(QComboBox* _cbo, int _month)
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









