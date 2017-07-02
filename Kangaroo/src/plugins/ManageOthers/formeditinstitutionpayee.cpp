#include "formeditinstitutionpayee.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QFormLayout>
#include <QPushButton>
#include <QTabWidget>
#include <KangarooLib/model/payee.h>
#include <KangarooLib/model/institution.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/ui/widgets/pictureselector.h>
#include <KangarooLib/ui/core.h>

using namespace KLib;

FormEditInstitutionPayee::FormEditInstitutionPayee(EditType _type, QWidget *parent) :
    CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
    m_type(_type),
    m_institution(NULL),
    m_payee(NULL),
    m_addedId(Constants::NO_ID)
{
    loadUI();
}

FormEditInstitutionPayee::FormEditInstitutionPayee(KLib::Institution* _institution, QWidget *parent) :
    CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
    m_type(Edit_Institution),
    m_institution(_institution),
    m_payee(NULL),
    m_addedId(Constants::NO_ID)
{
    loadUI();
    loadData();
}


FormEditInstitutionPayee::FormEditInstitutionPayee(KLib::Payee* _payee, QWidget *parent) :
    CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
    m_type(Edit_Payee),
    m_institution(NULL),
    m_payee(_payee),
    m_addedId(Constants::NO_ID)
{
    loadUI();
    loadData();
}


void FormEditInstitutionPayee::accept()
{
    //Only Name is a required field
    if (m_txtName->text().isEmpty())
    {
        QString msgPayee = tr("Please enter a name for the payee."),
                msgIns   = tr("Please enter a name for the institution.");
        QMessageBox::information(this,
                                 tr("Save Changes"),
                                 m_type == Edit_Payee ? msgPayee : msgIns);
        m_txtName->setFocus();
        return;
    }
    try
    {
        if (m_type == Edit_Payee)
        {
            if (!m_payee) //Add
            {
                m_payee = PayeeManager::instance()->add(m_txtName->text(), m_pictureSelector->idPicture());
                m_addedId = m_payee->id();
                m_payee->holdToModify();
            }
            else
            {
                m_payee->holdToModify();
                m_payee->setName(m_txtName->text());
                m_payee->setIdPicture(m_pictureSelector->idPicture());
            }

            m_payee->setAddress(m_txtAddress->toPlainText());
            m_payee->setCountry(m_cboCountry->currentText());
            m_payee->setPhone(m_txtPhone->text());
            m_payee->setFax(m_txtFax->text());
            m_payee->setContactName(m_txtContactName->text());
            m_payee->setWebsite(m_txtWebsite->text());
            m_payee->setNote(m_txtNote->toPlainText());

            m_payee->doneHoldToModify();
        }
        else //Institution
        {
            if (!m_institution) //Add
            {
                m_institution = InstitutionManager::instance()->add(m_txtName->text(), m_pictureSelector->idPicture());
                m_addedId = m_institution->id();
                m_institution->holdToModify();
            }
            else
            {
                m_institution->holdToModify();
                m_institution->setName(m_txtName->text());
                m_institution->setIdPicture(m_pictureSelector->idPicture());
            }

            m_institution->setAddress(m_txtAddress->toPlainText());
            m_institution->setCountry(m_cboCountry->currentText());
            m_institution->setPhone(m_txtPhone->text());
            m_institution->setFax(m_txtFax->text());
            m_institution->setContactName(m_txtContactName->text());
            m_institution->setWebsite(m_txtWebsite->text());
            m_institution->setInstitutionNumber(m_txtInstitutionNumber->text());
            m_institution->setRoutingNumber(m_txtRoutingNumber->text());
            m_institution->setNote(m_txtNote->toPlainText());

            m_institution->doneHoldToModify();
        }

        done(QDialog::Accepted);
    }
    catch (ModelException e)
    {
        QMessageBox::warning(this,
                             tr("Save Changes"),
                             tr("An error has occured while saving the data:\n\n%1").arg(e.description()));
    }
}

void FormEditInstitutionPayee::loadUI()
{
    QTabWidget* tabs = new QTabWidget(this);
    setCentralWidget(tabs);

    QWidget* mainTab = new QWidget(this);
    tabs->addTab(mainTab, tr("General"));

    m_pictureSelector = new PictureSelector(this);
    m_txtName = new QLineEdit(this);
    m_txtAddress = new QTextEdit(this);
    m_cboCountry = new QComboBox(this);
    m_txtPhone = new QLineEdit(this);
    m_txtFax = new QLineEdit(this);
    m_txtContactName = new QLineEdit(this);
    m_txtWebsite = new QLineEdit(this);
    m_txtNote = new QTextEdit(this);

    if (m_type == Edit_Institution)
    {
        m_txtInstitutionNumber = new QLineEdit(this);
        m_txtRoutingNumber = new QLineEdit(this);
        m_chkActive = new QCheckBox(tr("Is Acti&ve"), this);
        m_pictureSelector->setDefaultPicture(Core::pixmap("institution"));
        m_cboCountry->addItems(InstitutionManager::instance()->countries());

        m_chkActive->setChecked(true);

        if (m_institution)
        {
            setBothTitles(tr("Edit Institution"));
        }
        else
        {
            setBothTitles(tr("New Institution"));
        }
    }
    else
    {
        m_pictureSelector->setDefaultPicture(Core::pixmap("payee"));
        m_cboCountry->addItems(PayeeManager::instance()->countries());

        if (m_payee)
        {
            setBothTitles(tr("Edit Payee"));
        }
        else
        {
            setBothTitles(tr("New Payee"));
        }
    }

    m_cboCountry->setEditable(true);
    m_cboCountry->setCurrentText("");

    //Picture
    setPictureLabel(m_pictureSelector);

    QPushButton* btnSelectPicture = new QPushButton(Core::icon("picture"), tr("Pictu&re"), this);
    addButton(btnSelectPicture, 0, AtLeft);

    //Main Form
    QFormLayout* formLayout = new QFormLayout(mainTab);
    formLayout->addRow(tr("&Name:"), m_txtName);
    formLayout->addRow(tr("&Address:"), m_txtAddress);
    formLayout->addRow(tr("Co&untry:"), m_cboCountry);
    formLayout->addRow(tr("&Phone:"), m_txtPhone);
    formLayout->addRow(tr("Fa&x:"), m_txtFax);
    formLayout->addRow(tr("Con&tact Name:"), m_txtContactName);
    formLayout->addRow(tr("&Website:"), m_txtWebsite);

    if (m_type == Edit_Institution)
    {
        formLayout->addRow(tr("&Institution Number:"), m_txtInstitutionNumber);
        formLayout->addRow(tr("Routin&g Number:"), m_txtRoutingNumber);
        formLayout->addWidget(m_chkActive);
    }

    //Notes tab
    tabs->addTab(m_txtNote, Core::icon("note"), tr("Note"));

    connect(btnSelectPicture, &QPushButton::clicked, m_pictureSelector, &PictureSelector::selectPicture);

    m_txtName->setFocus();
}

void FormEditInstitutionPayee::loadData()
{
    if (m_institution)
    {
        m_txtName->setText(m_institution->name());
        m_txtAddress->setText(m_institution->address());
        m_cboCountry->setCurrentIndex(m_cboCountry->findText(m_institution->country()));
        m_txtPhone->setText(m_institution->phone());
        m_txtFax->setText(m_institution->fax());
        m_txtContactName->setText(m_institution->contactName());
        m_txtWebsite->setText(m_institution->website());
        m_txtInstitutionNumber->setText(m_institution->institutionNumber());
        m_txtRoutingNumber->setText(m_institution->routingNumber());
        m_chkActive->setChecked(m_institution->isActive());
        m_txtNote->setText(m_institution->note());
        m_pictureSelector->setIdPicture(m_institution->idPicture());
    }
    else //Payee
    {
        m_txtName->setText(m_payee->name());
        m_txtAddress->setText(m_payee->address());
        m_cboCountry->setCurrentIndex(m_cboCountry->findText(m_payee->country()));
        m_txtPhone->setText(m_payee->phone());
        m_txtFax->setText(m_payee->fax());
        m_txtContactName->setText(m_payee->contactName());
        m_txtWebsite->setText(m_payee->website());
        m_txtNote->setText(m_payee->note());
        m_pictureSelector->setIdPicture(m_payee->idPicture());
    }
}
