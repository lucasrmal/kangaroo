#ifndef LEDGERSETTINGS_H
#define LEDGERSETTINGS_H

#include "../../settingsmanager.h"

class QComboBox;
class QSpinBox;
class QCheckBox;

namespace KLib
{

    class LedgerSettings : public ISettingsPage
    {
        Q_OBJECT

        public:
            LedgerSettings(QWidget* parent = nullptr);

            QStringList validate() override;

            void save() override;
            void load() override;

        private:
            QSpinBox* m_spinAccountHeight;
            QCheckBox* m_chkAskBeforeNewPayee;
    };

    class LedgerScheduleSettings : public ISettingsPage
    {
        Q_OBJECT

        public:
            LedgerScheduleSettings(QWidget* parent = nullptr);

            QStringList validate() override;

            void save() override;
            void load() override;

        private slots:
            void onDisplayTypeChanged(int _row);

        private:
            QComboBox* m_scheduleDisplay;
            QSpinBox*  m_scheduleDays;
            QSpinBox*  m_scheduleInstances;
    };

}

#endif // LEDGERSETTINGS_H
