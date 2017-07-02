#include "formeditcurrency.h"
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/amount.h>
#include <KangarooLib/controller/onlinequotes.h>

#include <QComboBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QMessageBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QTabWidget>
#include <QLabel>

using namespace KLib;

FormEditCurrency::FormEditCurrency(KLib::Currency* _currency, QWidget *parent) :
    CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
    m_currency(_currency),
    m_mainCur(Account::getTopLevel()->mainCurrency()),
    m_addedSecurityId(Constants::NO_ID)
{
    QTabWidget* tabs = new QTabWidget(this);
    QWidget* mainTab = new QWidget(this);
    QWidget* quoteTab = new QWidget(this);
    setCentralWidget(tabs);

    m_txtCode   = new QLineEdit(this);
    m_txtName   = new QLineEdit(this);
    m_txtSymbol = new QLineEdit(this);
    m_spinPrec  = new QSpinBox(this);

    m_chkDownloadQuotes = new QCheckBox(tr("Enable online quotes for this currency"), this);
    m_cboQuoteSources = new QComboBox(this);
    QLabel* lblQuotes = new QLabel(tr("Quotes will be downloaded the next time you run the <i>Update Prices</i> tool."));
    lblQuotes->setWordWrap(true);

    connect(m_txtCode, &QLineEdit::textEdited, this, &FormEditCurrency::toUpper);
    connect(m_chkDownloadQuotes, &QCheckBox::toggled, this, &FormEditCurrency::onEnableQuotesChanged);

    QFormLayout* layout = new QFormLayout(mainTab);
    layout->addRow(tr("&Code:"), m_txtCode);
    layout->addRow(tr("&Name:"), m_txtName);
    layout->addRow(tr("&Symbol:"), m_txtSymbol);
    layout->addRow(tr("&Precision:"), m_spinPrec);

    QFormLayout* layoutQuotes = new QFormLayout(quoteTab);
    layoutQuotes->addRow("", m_chkDownloadQuotes);
    layoutQuotes->addRow(tr("&Quote Source:"), m_cboQuoteSources);
    layoutQuotes->addRow("", lblQuotes);

    setPicture(Core::pixmap("currency"));

    m_spinPrec->setMinimum(0);
    m_spinPrec->setMaximum(20);

    m_txtCode->setMaxLength(Amount::MAX_PRECISION);
    m_chkDownloadQuotes->setChecked(true);

    tabs->addTab(mainTab, tr("General"));
    tabs->addTab(quoteTab, Core::icon("internet"), tr("Online Quotes"));

    if (m_currency)
    {
        m_txtName->setText(m_currency->name());
        m_txtCode->setText(m_currency->code());
        m_txtSymbol->setText(m_currency->customSymbol());
        m_spinPrec->setValue(m_currency->precision());

        m_txtCode->setReadOnly(true);

        setTitle("Edit Currency");
        m_txtName->setFocus();

        if (m_currency->code() != m_mainCur)
        {
            try
            {
                ExchangePair* p = PriceManager::instance()->get(m_currency->code(),
                                                                m_mainCur);

                m_chkDownloadQuotes->setChecked(p->autoUpdate());

                if (!p->updateSource().isEmpty())
                {
                    m_cboQuoteSources->setCurrentIndex(m_cboQuoteSources->findData(p->updateSource()));
                }
                else
                {
                    m_cboQuoteSources->setCurrentIndex(m_cboQuoteSources->findData(OnlineQuotes::instance()->getDefault()->id()));
                }
            }
            catch (...) {}
        }
        else
        {
            tabs->removeTab(1);
        }
    }
    else
    {
        m_spinPrec->setValue(2);

        setTitle("New Currency");
        m_txtCode->setFocus();
    }

    //Load quote sources
    for (IQuote* q : OnlineQuotes::instance()->quoteSources())
    {
        m_cboQuoteSources->addItem(q->name(), q->id());
    }

    setWindowTitle(title());
}

void FormEditCurrency::onEnableQuotesChanged(bool enable)
{
    m_cboQuoteSources->setEnabled(enable);
}

void FormEditCurrency::toUpper(const QString& text)
{
    QLineEdit *le = qobject_cast<QLineEdit *>(sender());
    if (!le)
        return;
    le->setText(text.toUpper());
}

void FormEditCurrency::accept()
{
    QStringList errors;

    // Validate
    if (m_txtName->text().isEmpty())
    {
        errors << tr("The name is empty.");
    }

    if (m_txtCode->text().length() != 3)
    {
        errors << tr("The code must have length 3.");
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
                                 tr("The following errors prevent the currency to be "
                                    "saved:\n %1\nPress Yes to continue editing or "
                                    "Discard to discard the changes.").arg(strErrors),
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    try
    {
        if (m_currency)
        {
            m_currency->holdToModify();
            m_currency->setName(m_txtName->text());
            m_currency->setCustomSymbol(m_txtSymbol->text());
            m_currency->setPrecision(m_spinPrec->value());
            m_currency->doneHoldToModify();
        }
        else
        {
            m_currency = CurrencyManager::instance()->add(m_txtCode->text(),
                                                          m_txtName->text(),
                                                          m_txtSymbol->text(),
                                                          m_spinPrec->value());
            m_addedSecurityId = m_currency->id();
        }

        //Quotes
        if (m_currency->code() != m_mainCur)
        {
            ExchangePair* to = PriceManager::instance()->getOrAdd(m_mainCur, m_currency->code()),
                        * from = PriceManager::instance()->getOrAdd(m_currency->code(), m_mainCur);

            to->setAutoUpdate(m_chkDownloadQuotes->isChecked());
            from->setAutoUpdate(m_chkDownloadQuotes->isChecked());
            to->setUpdateSource(m_cboQuoteSources->currentIndex() != -1 ? m_cboQuoteSources->currentData().toString()
                                                                        : QString());
            from->setUpdateSource(m_cboQuoteSources->currentIndex() != -1 ? m_cboQuoteSources->currentData().toString()
                                                                          : QString());
        }

        done(QDialog::Accepted);
    }
    catch (ModelException e)
    {
        QMessageBox::warning(this,
                             tr("Save Changes"),
                             tr("An error has occured while saving the currency:\n\n%1").arg(e.description()));
    }
}
