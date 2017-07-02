#ifndef FORMEDITSCHEDULE_H
#define FORMEDITSCHEDULE_H

#include "camsegdialog.h"

class QLineEdit;
class QRadioButton;
class QDateEdit;
class QSpinBox;
class QStackedWidget;
class QComboBox;
class QCheckBox;

namespace KLib
{
    class Schedule;

    class FormEditSchedule : public CAMSEGDialog
    {
        Q_OBJECT

        enum FrequencyWidgets
        {
            Widget_Once,
            Widget_Day,
            Widget_Week,
            Widget_Month,
            Widget_Year,

            NUM_WIDGETS
        };

        public:
            explicit FormEditSchedule(Schedule* _schedule, QWidget *parent = nullptr);

            static const int MAX_INSTANCES_DATE = 5;

        public slots:
            void accept();

        private slots:
            void onStoppingChanged();
            void onFrequencyChanged();
            void onMonthChanged();

            void addInstance();
            void removeInstance();
            void loadData();

            void onReminderToggled(bool _enabled);

        private:
            void setupUI();
            void fillFrequencies();

            void showDaysOfMonth(int _numToShow);
            void hideDayOfMonth(int _index);

            static void setDaysInMonth(QComboBox* _cbo, int _month);

            QLineEdit* m_txtName;

            QRadioButton* m_optNotStopping;
            QRadioButton* m_optEndDate;
            QRadioButton* m_optNumRemaining;
            QDateEdit* m_dteStarts;
            QSpinBox* m_spinRemaining;
            QDateEdit* m_dteEndDate;
            QComboBox* m_cboFrequency;
            QLabel* m_lblReccurence;
            QLabel* m_lblEveryType;
            QStackedWidget* m_layoutFrequency;

            QCheckBox* m_chkRemind;
            QSpinBox* m_spinRemindBefore;

            //Daily
            QSpinBox* m_spinEvery;

            //Weekly
            QCheckBox* m_chkDayOfWeek[7];

            //Monthly
            QComboBox* m_cboDaysOfMonth[MAX_INSTANCES_DATE];
            QWidget* m_spacer[MAX_INSTANCES_DATE];

            int m_daysOfMonthShown;

            //Yearly
            QComboBox* m_cboMonth;
            QComboBox* m_cboDayOfMonth;

            QPushButton* m_btnAddInstance;
            QPushButton* m_btnRemoveInstance[MAX_INSTANCES_DATE];

            //Data
            Schedule* m_schedule;

    };

}

#endif // FORMEDITSCHEDULE_H
