#include "ledgersettings.h"

#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QFormLayout>
#include "model/schedule.h"

namespace KLib
{

    LedgerSettings::LedgerSettings(QWidget* parent) : ISettingsPage(parent)
    {
        m_spinAccountHeight = new QSpinBox(this);
        m_chkAskBeforeNewPayee = new QCheckBox(tr("Ask before adding new payee"), this);

        m_spinAccountHeight->setMinimum(1);
        m_spinAccountHeight->setMaximum(15);

        QFormLayout* layout = new QFormLayout(this);
        layout->addRow(tr("Number of account levels to display:"), m_spinAccountHeight);
        layout->addRow(m_chkAskBeforeNewPayee);
    }

    QStringList LedgerSettings::validate()
    {
        return {};
    }

    void LedgerSettings::save()
    {
        SettingsManager::instance()->setValue("Ledger/AccountHeightDisplayed", m_spinAccountHeight->value());
        SettingsManager::instance()->setValue("Ledger/AskBeforeAddingNewPayee", m_chkAskBeforeNewPayee->isChecked());

    }

    void LedgerSettings::load()
    {
        m_spinAccountHeight->setValue(SettingsManager::instance()->value("Ledger/AccountHeightDisplayed").toInt());
        m_chkAskBeforeNewPayee->setChecked(SettingsManager::instance()->value("Ledger/AskBeforeAddingNewPayee").toBool());
    }

    LedgerScheduleSettings::LedgerScheduleSettings(QWidget* parent) : ISettingsPage(parent)
    {
        m_scheduleDisplay = new QComboBox(this);
        m_scheduleDays = new QSpinBox(this);
        m_scheduleInstances = new QSpinBox(this);

        m_scheduleDisplay->addItem(tr("Show all schedule instances in N days window"));
        m_scheduleDisplay->addItem(tr("Show next N schedule instances"));

        m_scheduleDays->setMinimum(1);
        m_scheduleDays->setMaximum(60);

        m_scheduleInstances->setMinimum(1);
        m_scheduleInstances->setMaximum(Schedule::MAX_FUTURE_ENTERED);

        QFormLayout* layout = new QFormLayout(this);
        layout->addRow(tr("Schedule Display Type:"), m_scheduleDisplay);
        layout->addRow(tr("Number of days in future:"), m_scheduleDays);
        layout->addRow(tr("Number of instances in future:"), m_scheduleInstances);

        connect(m_scheduleDisplay, SIGNAL(currentIndexChanged(int)), this, SLOT(onDisplayTypeChanged(int)));
    }

    void LedgerScheduleSettings::onDisplayTypeChanged(int _row)
    {
        m_scheduleDays->setEnabled(_row == 0);
        m_scheduleInstances->setEnabled(_row == 1);
    }

    QStringList LedgerScheduleSettings::validate()
    {
        return {};
    }

    void LedgerScheduleSettings::save()
    {
        SettingsManager::instance()->setValue("Ledger/ScheduleDisplayPolicy", m_scheduleDisplay->currentIndex());
        SettingsManager::instance()->setValue("Ledger/ScheduleDisplayDays", m_scheduleDays->value());
        SettingsManager::instance()->setValue("Ledger/ScheduleDisplayInstances", m_scheduleInstances->value());
    }

    void LedgerScheduleSettings::load()
    {

        m_scheduleDisplay->setCurrentIndex(SettingsManager::instance()->value("Ledger/ScheduleDisplayPolicy").toInt());
        m_scheduleDays->setValue(SettingsManager::instance()->value("Ledger/ScheduleDisplayDays").toInt());
        m_scheduleInstances->setValue(SettingsManager::instance()->value("Ledger/ScheduleDisplayInstances").toInt());

        onDisplayTypeChanged(m_scheduleDisplay->currentIndex());
    }

}

