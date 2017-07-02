#ifndef FORMEDITRATE_H
#define FORMEDITRATE_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>

class QDoubleSpinBox;
class QDateEdit;
class QComboBox;
class QAbstractItemModel;

namespace KLib
{
    class ExchangePair;
}

class FormAddRate : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:
        explicit FormAddRate(QWidget *parent = 0);

        QSize sizeHint() const { return QSize(380, 300); }

        void selectPair(KLib::ExchangePair* _pair);

    public slots:
        void accept();
        void typeChanged();
        void fromToChanged();
        void fromChanged();
        void priceEdited();

    private:
        QComboBox* m_cboType;
        QComboBox* m_cboFrom;
        QComboBox* m_cboTo;
        QDateEdit* m_dteData;
        QDoubleSpinBox* m_spinValue;

        QAbstractItemModel* m_fromSec;
        QAbstractItemModel* m_fromCur;

        bool m_priceEdited;

};

#endif // FORMEDITRATE_H
