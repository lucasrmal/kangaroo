#ifndef FORMEDITACCOUNT_H
#define FORMEDITACCOUNT_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>
#include <functional>

namespace KLib
{
    class Account;
    class AccountSelector;
    class PictureSelector;
    class AmountEdit;
}

class QLineEdit;
class QComboBox;
class QCheckBox;
class QTextEdit;
class QTabWidget;
class AccountEditTab;


class FormEditAccount : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:
        typedef std::function<AccountEditTab*(QWidget*)> fn_buildTab;

        explicit FormEditAccount(KLib::Account* _account, QWidget* _parent = nullptr);

        void setParentAccount(KLib::Account* _parent);

        static void registerTab(fn_buildTab _buildTab);

        QSize sizeHint() const override;


    public slots:
        void accept();

    private slots:
        void onParentAccountChanged(KLib::Account* _account);
        void onAccountTypeChanged();
        void showCreditLimit();

        void addSecurity();
        void addInstitution();

        void updateTitle(const QString& _title);

    private:
        void fillData();

        KLib::Account* m_account;
        KLib::PictureSelector* m_pictureSelector;

        QTabWidget* m_tabs;
        QWidget* m_mainTab;
        QLineEdit* m_txtName;
        QLineEdit* m_txtCode;
        QComboBox* m_cboCurrency;
        QComboBox* m_cboSecurity;
        QPushButton* m_btnAddSecurity;
        QPushButton* m_btnAddInstitution;
        QComboBox* m_cboInstitution;
        KLib::AccountSelector* m_cboParent;
        QComboBox* m_cboType;
        QCheckBox* m_chkPlaceholder;
        QTextEdit* m_txtNote;
        QLabel* m_lblCreditLimit;
        KLib::AmountEdit* m_txtCreditLimit;

        QTimer*    m_timer;

        QList<AccountEditTab*> m_extraTabs;

        static QList<fn_buildTab> m_registeredTabs;



};

#endif // FORMEDITACCOUNT_H
