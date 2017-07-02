#ifndef SCHEDULEENTRYFORM_H
#define SCHEDULEENTRYFORM_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>
#include <KangarooLib/model/transaction.h>

namespace KLib
{
    class Schedule;
    class SplitsWidget;
}

class QLineEdit;
class QComboBox;
class QDateEdit;
class QGroupBox;

class ScheduleEntryForm : public KLib::CAMSEGDialog
{
    Q_OBJECT

        struct TempTransaction
        {
            QString no;
            QString memo;
            QString payee;
            QDate date;
            QList<KLib::Transaction::Split> splits;
        };

    public:
        explicit ScheduleEntryForm(const QList<KLib::Schedule*>& _schedules,
                                   const QList<QDate>& _dates,
                                   QWidget *parent = 0);

    public slots:
        void showPrevious();
        void showNext();
        void enterCurrent();

    private:
        void loadSchedule(int _idx, bool _save = true);

        QGroupBox* m_frmSchedule;
        QLineEdit* m_txtNo;
        QLineEdit* m_txtMemo;
        QComboBox* m_cboPayee;
        QDateEdit* m_dteDate;

        KLib::SplitsWidget* m_splitsWidget;
        QList<KLib::Transaction::Split> m_splits;

        QPushButton* m_btnPrevious;
        QPushButton* m_btnNext;
        QPushButton* m_btnEnter;

        QList<KLib::Schedule*> m_schedules;
        QList<TempTransaction> m_transactions;

        int m_currentIdx;

};

#endif // SCHEDULEENTRYFORM_H
