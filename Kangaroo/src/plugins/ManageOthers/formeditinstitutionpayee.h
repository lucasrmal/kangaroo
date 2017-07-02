#ifndef FORMEDITINSTITUTIONPAYEE_H
#define FORMEDITINSTITUTIONPAYEE_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>

class QLineEdit;
class QTextEdit;
class QCheckBox;
class QComboBox;

namespace KLib
{
    class Institution;
    class Payee;
    class PictureSelector;
}

class FormEditInstitutionPayee : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:

        enum EditType
        {
            Edit_Institution,
            Edit_Payee
        };

        /**
         * @brief Constructor to add a new object
         * @param _type The type, institution or payee
         * @param parent
         * @see EditType
         */
        explicit FormEditInstitutionPayee(EditType _type, QWidget *parent = 0);

        /**
         * @brief To edit an existing institution
         * @param _institution
         * @param parent
         */
        explicit FormEditInstitutionPayee(KLib::Institution* _institution, QWidget *parent = 0);

        /**
         * @brief To edit an existing payee
         * @param _payee
         * @param parent
         */
        explicit FormEditInstitutionPayee(KLib::Payee* _payee, QWidget *parent = 0);

        QSize sizeHint() const { return QSize(450, 350); }

        /**
         * @brief Returns the id of the added object, after it is added.
         * @return
         */
        int addedId() const { return m_addedId; }

    public slots:
        void accept();

    private:
        void loadUI();
        void loadData();

        EditType m_type;
        KLib::Institution* m_institution;
        KLib::Payee* m_payee;
        int m_addedId;

        QLineEdit* m_txtName;
        QCheckBox* m_chkActive;
        QTextEdit* m_txtAddress;
        QComboBox* m_cboCountry;
        QLineEdit* m_txtPhone;
        QLineEdit* m_txtFax;
        QLineEdit* m_txtContactName;
        QLineEdit* m_txtWebsite;
        QLineEdit* m_txtInstitutionNumber;
        QLineEdit* m_txtRoutingNumber;
        QTextEdit* m_txtNote;

        KLib::PictureSelector* m_pictureSelector;


};

#endif // FORMEDITINSTITUTIONPAYEE_H
