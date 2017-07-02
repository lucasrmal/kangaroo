#ifndef FORMEDITCURRENCY_H
#define FORMEDITCURRENCY_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>

class QTabWidget;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QComboBox;

namespace KLib
{
    class Currency;
}

class FormEditCurrency : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:
        explicit FormEditCurrency(KLib::Currency* _currency, QWidget *parent = nullptr);

        QSize sizeHint() const { return QSize(400, 400); }

        KLib::Currency* currency() { return m_currency; }

        int addedCurrencyId() const { return m_addedSecurityId; }

    signals:

    public slots:
        void accept();
        void onEnableQuotesChanged(bool enable);

    private slots:
        void toUpper(const QString& text);

    private:
        KLib::Currency* m_currency;
        QString m_mainCur;

        QTabWidget* m_tabs;

        //Main Tab
        QLineEdit* m_txtCode;
        QLineEdit* m_txtName;
        QLineEdit* m_txtSymbol;
        QSpinBox*  m_spinPrec;

        //Quote Tab
        QComboBox* m_cboQuoteSources;
        QCheckBox* m_chkDownloadQuotes;

        int m_addedSecurityId;

};



#endif // FORMEDITCURRENCY_H
