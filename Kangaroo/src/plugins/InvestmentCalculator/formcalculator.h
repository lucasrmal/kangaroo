#ifndef FORMCALCULATOR_H
#define FORMCALCULATOR_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>

class QLabel;
class QSpinBox;
class QComboBox;

namespace KLib
{
    class AmountEdit;
}

class FormCalculator : public KLib::CAMSEGDialog
{
    Q_OBJECT

    enum Period
    {
        Year,
        Month,
        Day
    };

    public:
        explicit FormCalculator(QWidget *parent = 0);

        static const int PRECISION;

    protected:
        void keyPressEvent(QKeyEvent* _event);

    public slots:
        void calculate();

    private:
        KLib::AmountEdit* m_txtInitial;
        KLib::AmountEdit* m_txtRecurring;
        QComboBox* m_cboRecurring;

        QSpinBox* m_spinPeriods;
        QComboBox* m_cboPeriods;
        QComboBox* m_cboCompounded;
        KLib::AmountEdit* m_txtInterest;

        QLabel* m_lblPrincipal;
        QLabel* m_lblInterest;
        QLabel* m_lblTotal;


};

#endif // FORMCALCULATOR_H
