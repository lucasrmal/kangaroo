#ifndef FORMEDITSECURITY_H
#define FORMEDITSECURITY_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>

class QComboBox;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QTextEdit;

namespace KLib
{
    class Security;
    class PictureSelector;
}

class FormEditSecurity : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:

        enum class SecurityClass
        {
            Security,
            Index
        };

        FormEditSecurity(SecurityClass _class, KLib::Security* _security, QWidget *parent = nullptr);

        QSize sizeHint() const { return QSize(420, 300); }

        int addedSecurityId() const { return m_addedSecurityId; }

    public slots:
        void accept();

    private slots:
        void toUpper(const QString& text);

        void onTypeChanged();
        void onEnableQuotesChanged(bool enable);

        void addCurrency();
        void addProvider();

    private:
        KLib::Security* m_security;
        SecurityClass m_class;

        //Main Tab
        QComboBox* m_cboType;
        QLineEdit* m_txtName;
        QComboBox* m_cboMarket;
        QComboBox* m_cboSector;
        QLineEdit* m_txtSymbol;
        QComboBox* m_cboCurrency;
        QComboBox* m_cboProvider;
        QSpinBox*  m_spinPrec;
        QCheckBox* m_defaultPicture;
        KLib::PictureSelector* m_pictureSelector;
        QTimer*    m_timer;

        //Quote Tab
        QComboBox* m_cboQuoteSources;
        QCheckBox* m_chkDownloadQuotes;

        //Note Tab
        QTextEdit* m_txtNote;

        int m_addedSecurityId;

};

#endif // FORMEDITSECURITY_H
