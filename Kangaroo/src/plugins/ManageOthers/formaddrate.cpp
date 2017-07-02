#include "formaddrate.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/controller/securitycontroller.h>
#include <KangarooLib/controller/currencycontroller.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/model/pricemanager.h>
#include <KangarooLib/model/security.h>
#include <QFormLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QSortFilterProxyModel>
#include <QMessageBox>

using namespace KLib;

FormAddRate::FormAddRate(QWidget *parent) :
    CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
    m_priceEdited(false)
{
    setPicture(Core::pixmap("price"));
    setTitle(tr("Add Price"));
    setWindowTitle(title());

    QFormLayout* layout = new QFormLayout(centralWidget());

    m_cboType   = new QComboBox(this);
    m_cboFrom   = new QComboBox(this);
    m_cboTo     = new QComboBox(this);
    m_dteData   = new QDateEdit(this);
    m_spinValue = new QDoubleSpinBox(this);

    layout->addRow(tr("T&ype:"), m_cboType);
    layout->addRow(tr("&From:"), m_cboFrom);
    layout->addRow(tr("&To:"), m_cboTo);
    layout->addRow(tr("&Date:"), m_dteData);
    layout->addRow(tr("&Rate:"), m_spinValue);

    m_spinValue->setDecimals(PriceManager::DECIMALS_RATE);
    m_spinValue->setMinimum(0);
    m_spinValue->setMaximum(qInf());

    Core::setupDateEdit(m_dteData);
    m_dteData->setDate(QDate::currentDate());

    m_fromSec = SecurityController::sortProxy(this);
    m_fromCur = CurrencyController::sortProxy(this);

    m_cboTo->setModel(CurrencyController::sortProxy(this));

    m_cboType->addItem(tr("Currency"));
    m_cboType->addItem(tr("Security"));
    connect(m_cboType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged()));

    connect(m_cboFrom, SIGNAL(currentIndexChanged(int)), this, SLOT(fromToChanged()));
    connect(m_cboFrom, SIGNAL(currentIndexChanged(int)), this, SLOT(fromChanged()));
    connect(m_cboTo, SIGNAL(currentIndexChanged(int)), this, SLOT(fromToChanged()));
    connect(m_cboType, SIGNAL(currentIndexChanged(int)), this, SLOT(fromToChanged()));

    connect(m_spinValue, SIGNAL(editingFinished()), this, SLOT(priceEdited()));

    centralWidget()->setLayout(layout);

    typeChanged();
    fromToChanged();

}

void FormAddRate::selectPair(KLib::ExchangePair* _pair)
{
    if (!_pair->isSecurity())
    {
        m_cboType->setCurrentIndex(0);
        m_cboFrom->setCurrentIndex(m_cboFrom->findData(_pair->from()));
    }
    else
    {
        m_cboType->setCurrentIndex(1);
        m_cboFrom->setCurrentIndex(m_cboFrom->findData(_pair->securityFrom()->id()));
    }

    m_cboTo->setCurrentIndex(m_cboTo->findData(_pair->to()));
}

void FormAddRate::typeChanged()
{
    if (m_cboType->currentIndex() == 0)
    {
        m_cboFrom->setModel(m_fromCur);
    }
    else
    {
        m_cboFrom->setModel(m_fromSec);
        m_cboFrom->setModelColumn(SecurityColumn::NAME);
    }
}

void FormAddRate::fromToChanged()
{
    if (!m_priceEdited)
    {
        QString from = m_cboFrom->currentData().toString();
        if (m_cboType->currentIndex() == 1)
            from = "SEC" + from;

        try
        {
        m_spinValue->setValue(PriceManager::instance()->get(from,
                                                            m_cboTo->currentData().toString())->last());
        }
        catch (ModelException)
        {}
    }
}

void FormAddRate::fromChanged()
{
    try
    {
        if (m_cboType->currentIndex() == 1) // Security
        {
            m_cboTo->setCurrentIndex(
                        m_cboTo->findData(SecurityManager::instance()->get(m_cboFrom->currentData().toInt())->currency()));
        }
    }
    catch (ModelException)
    {}
}

void FormAddRate::priceEdited()
{
    m_priceEdited = true;
}

void FormAddRate::accept()
{
    QStringList errors;
    // Validate

    if (m_cboFrom->currentIndex() == -1)
    {
        errors << tr("The \"from\" field is invalid.");
    }

    if (m_cboTo->currentIndex() == -1)
    {
        errors << tr("The \"to\" field is invalid.");
    }

    if (!m_dteData->date().isValid())
    {
        errors << tr("The date is invalid.");
    }

    if (m_cboType->currentIndex() == 0 && m_cboFrom->currentData().toString() == m_cboTo->currentData().toString())
    {
        errors << tr("The \"from\" and \"to\" currencies must be different.");
    }

    if (errors.count() > 0)
    {
        QString strErrors;
        for (QString s : errors)
        {
            strErrors += "\t" + s + "\n";
        }
        QMessageBox::information(this,
                                 tr("Save Changes"),
                                 tr("The following errors prevent the rate to be "
                                    "saved:\n %1\nPress Yes to continue editing or "
                                    "Discard to discard the changes.").arg(strErrors),
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    try
    {
        QString from = m_cboFrom->currentData().toString();
        if (m_cboType->currentIndex() == 1)
            from = "SEC" + from;

        PriceManager::instance()->getOrAdd(from,
                                           m_cboTo->currentData().toString())->set(m_dteData->date(),
                                                                                   m_spinValue->value());
        done(QDialog::Accepted);
    }
    catch (ModelException e)
    {
        QMessageBox::warning(this,
                             tr("Save Changes"),
                             tr("An error has occured while saving the rate:\n\n%1").arg(e.description()));
    }
}
