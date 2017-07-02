#ifndef DATEINTERVALSELECTOR_H
#define DATEINTERVALSELECTOR_H

#include <QWidget>
#include <QDate>

class QComboBox;
class QSpinBox;

namespace KLib
{
    typedef QPair<QDate, QDate> DateInterval;

    class DateIntervalSelector : public QWidget
    {
        Q_OBJECT

        public:

            enum Flags
            {
                Flag_ThroughToday = 0x01,
                Flag_AllTime = 0x02,
                Flag_YTD = 0x04,
                Flag_Year = 0x08,
                Flag_Month = 0x16,

                Flag_All = Flag_ThroughToday | Flag_AllTime | Flag_YTD | Flag_Year | Flag_Month,
                Flag_YearMonth = Flag_Year | Flag_Month
            };



            explicit DateIntervalSelector(int _flags, QWidget* parent = nullptr);

            DateInterval interval() const;

        signals:
            void intervalChanged(const DateInterval& _interval);

        public slots:
            void setYear(int _year);
            void setMonth(int _month, int _year);
            void setAllTime();
            void setThroughToday();
            void setYTD();


        private slots:
            void onChangeBalanceType(int _index);
            void onMonthOrYearChanged();

        private:
            int         m_flags;

            QComboBox*  m_cboBalanceSelector;
            QSpinBox*   m_spinYear;
            QComboBox*  m_cboMonth;
    };

}

#endif // DATEINTERVALSELECTOR_H
