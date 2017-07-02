#include "formeditsecurity.h"
#include "formeditcurrency.h"
#include "formeditinstitutionpayee.h"
#include <KangarooLib/amount.h>
#include <KangarooLib/model/security.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/controller/currencycontroller.h>
#include <KangarooLib/controller/institutioncontroller.h>
#include <KangarooLib/controller/onlinequotes.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/widgets/pictureselector.h>

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QCheckBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>

using namespace KLib;

FormEditSecurity::FormEditSecurity(SecurityClass _class, KLib::Security* _security, QWidget *parent) :
    CAMSEGDialog(DialogWithoutPicture, OkCancelButtons, parent),
    m_security(_security),
    m_class(_class),
    m_addedSecurityId(Constants::NO_ID)
{
    QTabWidget* tabs = new QTabWidget(this);
    QWidget* mainTab = new QWidget(tabs);
    QWidget* quoteTab = new QWidget(tabs);
    setCentralWidget(tabs);

    m_timer = new QTimer(this);
    m_pictureSelector = new PictureSelector(this);
    if (m_class != SecurityClass::Index) m_cboType = new QComboBox(mainTab);
    m_txtName = new QLineEdit(mainTab);
    if (m_class != SecurityClass::Index) m_cboMarket = new QComboBox(mainTab);
    m_cboSector = new QComboBox(mainTab);
    m_txtSymbol = new QLineEdit(mainTab);
    m_cboCurrency = new QComboBox(mainTab);
    if (m_class != SecurityClass::Index) m_cboProvider = new QComboBox(mainTab);
    m_spinPrec = new QSpinBox(mainTab);
    m_txtNote = new QTextEdit(mainTab);
    m_defaultPicture = new QCheckBox(tr("Default to Security Picture for Accounts"), mainTab);

    m_defaultPicture->setToolTip(tr("If checked and a picture is set for the security, \n"
                                    "investment accounts using this security for which \n"
                                    "no picture is set will use the security picture."));

    setPictureLabel(m_pictureSelector);

    m_chkDownloadQuotes = new QCheckBox(tr("Enable online quotes for this security"), this);
    m_cboQuoteSources = new QComboBox(this);
    QLabel* lblQuotes = new QLabel(tr("Quotes will be downloaded the next time you run the <i>Update Prices</i> tool."));
    lblQuotes->setWordWrap(true);

    QPushButton* btnSelectPicture = new QPushButton(Core::icon("picture"), tr("Pictu&re"), this);
    addButton(btnSelectPicture, 0, AtLeft);

    QPushButton* btnAddCurrency = new QPushButton(Core::icon("list-add"), "", this);
    btnAddCurrency->setMaximumWidth(24);

    QHBoxLayout* layCurr = new QHBoxLayout();
    layCurr->addWidget(m_cboCurrency);
    layCurr->addWidget(btnAddCurrency);

    QLabel* lblCurrency = new QLabel(tr("Trading &Currency:"), this);
    lblCurrency->setBuddy(m_cboCurrency);

    QFormLayout* layoutMain = new QFormLayout(mainTab);

    if (m_class != SecurityClass::Index)
        layoutMain->addRow(tr("&Type:"), m_cboType);

    layoutMain->addRow(tr("&Name:"), m_txtName);

    if (m_class != SecurityClass::Index)
        layoutMain->addRow(tr("&Market:"), m_cboMarket);

    layoutMain->addRow(tr("&Sector:"), m_cboSector);
    layoutMain->addRow(tr("S&ymbol:"), m_txtSymbol);
    layoutMain->addRow(lblCurrency, layCurr);

    if (m_class != SecurityClass::Index)
    {
        QPushButton* btnAddProvider = new QPushButton(Core::icon("list-add"), "", this);
        btnAddProvider->setMaximumWidth(24);

        QHBoxLayout* layProv = new QHBoxLayout();
        layProv->addWidget(m_cboProvider);
        layProv->addWidget(btnAddProvider);

        QLabel* lblProvider = new QLabel(tr("&Provider:"), this);
        lblProvider->setBuddy(m_cboProvider);

        layoutMain->addRow(lblProvider, layProv);

        connect(btnAddProvider, &QPushButton::clicked,  this, &FormEditSecurity::addProvider);
    }

    layoutMain->addRow(tr("P&recision:"), m_spinPrec);
    layoutMain->addRow("", m_defaultPicture);


    QFormLayout* layoutQuotes = new QFormLayout(quoteTab);
    layoutQuotes->addRow("", m_chkDownloadQuotes);
    layoutQuotes->addRow(tr("&Quote Source:"), m_cboQuoteSources);
    layoutQuotes->addRow("", lblQuotes);

    tabs->addTab(mainTab, tr("General"));
    tabs->addTab(quoteTab, Core::icon("internet"), tr("Online Quotes"));
    tabs->addTab(m_txtNote, Core::icon("note"), tr("Note"));

    if (m_class != SecurityClass::Index)
    {
        m_cboType->addItem(Security::typeToString(SecurityType::Stock),          (int) SecurityType::Stock);
        m_cboType->addItem(Security::typeToString(SecurityType::PreferredStock), (int) SecurityType::PreferredStock);
        m_cboType->addItem(Security::typeToString(SecurityType::ETF),            (int) SecurityType::ETF);
        m_cboType->addItem(Security::typeToString(SecurityType::MutualFund),     (int) SecurityType::MutualFund);
        m_cboType->addItem(Security::typeToString(SecurityType::Bond),           (int) SecurityType::Bond);

        m_cboMarket->setEditable(true);
        m_cboMarket->addItems(SecurityManager::instance()->markets());

        connect(m_cboMarket, SIGNAL(currentTextChanged(QString)), SLOT(toUpper(const QString &)));
        connect(m_cboType, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChanged()));

        m_cboProvider->setModel(InstitutionController::sortProxy(this));
    }

    m_cboSector->setEditable(true);
    m_cboSector->addItems(SecurityManager::instance()->sectors());

    m_cboCurrency->setModel(CurrencyController::sortProxy(this));
    m_cboCurrency->setModelColumn(1);

    m_spinPrec->setMinimum(0);
    m_spinPrec->setMaximum(Amount::MAX_PRECISION);

    //Load quote sources
    for (IQuote* q : OnlineQuotes::instance()->quoteSources())
    {
        m_cboQuoteSources->addItem(q->name(), q->id());
    }

    connect(btnAddCurrency,      &QPushButton::clicked,  this, &FormEditSecurity::addCurrency);
    connect(m_txtSymbol,         &QLineEdit::textEdited, this, &FormEditSecurity::toUpper);
    connect(m_chkDownloadQuotes, &QCheckBox::toggled,    this, &FormEditSecurity::onEnableQuotesChanged);
    connect(btnSelectPicture,    &QPushButton::clicked,  m_pictureSelector, &PictureSelector::selectPicture);

    m_pictureSelector->setDefaultPicture(Core::pixmap(m_class == SecurityClass::Index ? "index" : "security"));
    m_chkDownloadQuotes->setChecked(true);

    if (m_security)
    {
        if (m_class != SecurityClass::Index)
        {
            m_cboType->setCurrentIndex(m_cboType->findData((int) m_security->type()));
            m_cboMarket->setCurrentIndex(m_cboMarket->findText(m_security->market()));
            m_cboProvider->setCurrentIndex(m_cboProvider->findData(m_security->idProvider()));

            setBothTitles("Edit Security");
        }
        else
        {
            setBothTitles("Edit Index");
        }

        m_txtName->setText(m_security->name());
        m_cboSector->setCurrentIndex(m_cboSector->findText(m_security->sector()));
        m_txtSymbol->setText(m_security->symbol());
        m_cboCurrency->setCurrentIndex(m_cboCurrency->findData(m_security->currency()));
        m_spinPrec->setValue(m_security->precision());
        m_defaultPicture->setChecked(m_security->defaultToSecurityPicture());
        m_pictureSelector->setIdPicture(m_security->idPicture());
        m_txtNote->setText(m_security->note());

        try
        {
            ExchangePair* p = PriceManager::instance()->get(PriceManager::securityId(m_security->id()),
                                                            m_security->currency());

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
        if (m_class != SecurityClass::Index)
        {
            m_cboMarket->setCurrentText("");
            setBothTitles("New Security");
        }
        else
        {
            setBothTitles("New Index");
        }

        m_cboSector->setCurrentText("");
        m_spinPrec->setValue(2);
        m_defaultPicture->setChecked(true);
    }

    m_txtName->setFocus();
    onTypeChanged();
    onEnableQuotesChanged(m_chkDownloadQuotes->isChecked());
}

void FormEditSecurity::toUpper(const QString& text)
{
    QLineEdit *le = qobject_cast<QLineEdit *>(sender());
    if (!le)
        return;
    le->setText(text.toUpper());
}

void FormEditSecurity::onTypeChanged()
{
    if (m_class != SecurityClass::Index)
    {
        SecurityType type = (SecurityType) m_cboType->currentData().toInt();
        m_cboProvider->setEnabled(type == SecurityType::ETF || type == SecurityType::MutualFund);
    }
}

void FormEditSecurity::onEnableQuotesChanged(bool enable)
{
    m_cboQuoteSources->setEnabled(enable);
}

void FormEditSecurity::addCurrency()
{
    /*
     * We need a timer since m_cboCurrency uses a proxy model to sort the CurrencyController.
     * Because of that, the sort is done after setting the new index, which results in problems.
     * The timer bypasses that, setting the index after the sort is done.
     */

    FormEditCurrency* form = new FormEditCurrency(nullptr, this);
    int idCur = 0;
    if (form->exec() == QDialog::Accepted)
    {
        idCur = form->addedCurrencyId();
        m_timer->setInterval(500);

        connect(m_timer, &QTimer::timeout, [=]()
        {
            m_cboCurrency->setCurrentIndex(m_cboCurrency->findData(idCur));
            m_timer->stop();
            m_timer->disconnect();
        });
        m_timer->start();
    }

    delete form;
}

void FormEditSecurity::addProvider()
{
    /*
     * We need a timer since m_cboCurrency uses a proxy model to sort the CurrencyController.
     * Because of that, the sort is done after setting the new index, which results in problems.
     * The timer bypasses that, setting the index after the sort is done.
     */

    FormEditInstitutionPayee* form = new FormEditInstitutionPayee(FormEditInstitutionPayee::Edit_Institution, this);
    int idIns = 0;
    if (form->exec() == QDialog::Accepted)
    {
        idIns = form->addedId();
        m_timer->setInterval(500);

        connect(m_timer, &QTimer::timeout, [=]()
        {
            m_cboProvider->setCurrentIndex(m_cboProvider->findData(idIns));
            m_timer->stop();
            m_timer->disconnect();
        });
        m_timer->start();
    }

    delete form;
}

void FormEditSecurity::accept()
{
    QStringList errors;

    // Validate
    if (m_class != SecurityClass::Index && m_cboType->currentIndex() == -1)
    {
        errors << tr("The type is invalid.");
    }

    if (m_txtName->text().isEmpty())
    {
        errors << tr("The name is empty.");
    }

    if (m_txtSymbol->text().isEmpty())
    {
        errors << tr("The symbol is empty.");
    }

    if (m_cboCurrency->currentIndex() < 0)
    {
        errors << tr("The currency is invalid.");
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
                                 tr("The following errors prevent the security to be "
                                    "saved:\n %1\nPress Yes to continue editing or "
                                    "Discard to discard the changes.").arg(strErrors),
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    try
    {
        if (m_security)
        {
            m_security->holdToModify();
            m_security->setName(m_txtName->text());
            m_security->setSector(m_cboSector->currentText());
            m_security->setSymbol(m_txtSymbol->text());
            m_security->setCurrency(m_cboCurrency->currentData().toString());
            m_security->setPrecision(m_spinPrec->value());
            m_security->setIdPicture(m_pictureSelector->idPicture());
            m_security->setDefaultToSecurityPicture(m_defaultPicture->isChecked());
            m_security->setNote(m_txtNote->toPlainText());

            if (m_class != SecurityClass::Index)
            {
                m_security->setType((SecurityType) m_cboType->currentData().toInt());
                m_security->setMarket(m_cboMarket->currentText());
                m_security->setIdProvider(m_cboProvider->currentData().toInt());
            }

            m_security->doneHoldToModify();
        }
        else
        {
            m_security = SecurityManager::instance()
                         ->add(m_class != SecurityClass::Index ? (SecurityType) m_cboType->currentData().toInt()
                                                               : SecurityType::Index,
                               m_txtName->text(),
                               m_txtSymbol->text(),
                               m_class != SecurityClass::Index ? m_cboMarket->currentText() : QString(),
                               m_cboSector->currentText(),
                               m_cboCurrency->currentData().toString(),
                               m_spinPrec->value());

            m_addedSecurityId = m_security->id();

            m_security->holdToModify();
            m_security->setIdPicture(m_pictureSelector->idPicture());
            m_security->setDefaultToSecurityPicture(m_defaultPicture->isChecked());
            m_security->setNote(m_txtNote->toPlainText());

            if (m_class != SecurityClass::Index)
                m_security->setIdProvider(m_cboProvider->currentData().toInt());

            m_security->doneHoldToModify();
        }

        //Quotes
        ExchangePair* p = PriceManager::instance()->getOrAdd(PriceManager::securityId(m_security->id()),
                                                             m_security->currency());

        p->setAutoUpdate(m_chkDownloadQuotes->isChecked());
        p->setUpdateSource(m_cboQuoteSources->currentIndex() != -1 ? m_cboQuoteSources->currentData().toString()
                                                                   : QString());

        done(QDialog::Accepted);
    }
    catch (ModelException e)
    {
        QMessageBox::warning(this,
                             tr("Save Changes"),
                             tr("An error has occured while saving the security:\n\n%1").arg(e.description()));
    }
}
