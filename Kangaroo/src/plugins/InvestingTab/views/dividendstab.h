#ifndef DIVIDENDSTAB_H
#define DIVIDENDSTAB_H

#include "standardtab.h"

class DividendsModel;

namespace KLib
{
    class PercChart;
    class DateIntervalSelector;
}

class DividendsTab : public StandardTab
{
    Q_OBJECT

    public:
        explicit DividendsTab(Portfolio* _portfolio, QWidget *parent = nullptr);

    private:
        DividendsModel* m_model;

        KLib::PercChart* m_pieChart;
        KLib::PercChart* m_barChart;

        KLib::DateIntervalSelector*  m_dateSelector;
};

#endif // DIVIDENDSTAB_H
