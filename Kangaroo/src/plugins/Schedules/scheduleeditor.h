#ifndef SCHEDULEEDITOR_H
#define SCHEDULEEDITOR_H

#include <QWidget>
#include <KangarooLib/model/transaction.h>

class QTreeView;
class QGroupBox;
class QLineEdit;
class QCheckBox;
class QRadioButton;
class QSpinBox;
class QDateEdit;
class QComboBox;
class QPushButton;
class QStackedWidget;
class QLabel;

namespace KLib
{
    class SplitsWidget;
    class Schedule;
}

class ScheduleEditor : public QWidget
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
        explicit ScheduleEditor(QWidget *parent = 0);

        static const int MAX_INSTANCES_DATE = 5;

    private slots:
        void onStoppingChanged();
        void onFrequencyChanged();
        void onMonthChanged();
        void updateImbalances();

        void addNew();
        void loadSchedule(const QModelIndex& _index);
        bool saveChanges(bool _ask = false);
        void cancel();
        void clear();

        void addInstance();
        void removeInstance();

    private:
        void setupUI();
        void fillFrequencies();
        void reloadSchedule();

        void showDaysOfMonth(int _numToShow);
        void hideDayOfMonth(int _index);

        static void setDaysInMonth(QComboBox* _cbo, int _month);

        QTreeView* m_scheduleView;
        QGroupBox* m_frmSchedule;

        QLineEdit* m_txtName;
        QCheckBox* m_isActive;
        QCheckBox* m_autoEnter;

        QGroupBox* m_frmRecurrence;
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

        QGroupBox* m_frmTransaction;
        QLineEdit* m_txtMemo;
        QComboBox* m_cboPayee;
        KLib::SplitsWidget* m_splitEditor;
        QLabel* m_lblImbalances;

        QList<KLib::Transaction::Split> m_splits;

        QPushButton* m_btnAdd;
        QPushButton* m_btnRemove;
        QPushButton* m_btnCancel;
        QPushButton* m_btnSave;

        //bool m_isModified;
        KLib::Schedule* m_current;


};

#endif // SCHEDULEEDITOR_H
