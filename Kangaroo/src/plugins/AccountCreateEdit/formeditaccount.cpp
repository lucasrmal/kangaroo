#include "formeditaccount.h"
#include "accountedittab.h"
#include "../ManageOthers/formeditsecurity.h"
#include "../ManageOthers/formeditinstitutionpayee.h"
#include <KangarooLib/ui/core.h>
#include <KangarooLib/controller/currencycontroller.h>
#include <KangarooLib/controller/institutioncontroller.h>
#include <KangarooLib/controller/securitycontroller.h>
#include <KangarooLib/controller/accountcontroller.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/institution.h>
#include <KangarooLib/model/ledger.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/ui/widgets/accountselector.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/model/picturemanager.h>
#include <KangarooLib/ui/widgets/pictureselector.h>
#include <KangarooLib/ui/widgets/amountedit.h>

#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QTextEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QTabWidget>
#include <QProxyStyle>

using namespace KLib;

QList<FormEditAccount::fn_buildTab> FormEditAccount::m_registeredTabs = QList<FormEditAccount::fn_buildTab>();

class CustomTabStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (type == QStyle::CT_TabBarTab)
            s.transpose();
        return s;
    }

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
        if (element == CE_TabBarTabLabel)
        {
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
            {
                QStyleOptionTab opt(*tab);
                opt.shape = QTabBar::RoundedNorth;
                QProxyStyle::drawControl(element, &opt, painter, widget);
                return;
            }
        }
        QProxyStyle::drawControl(element, option, painter, widget);
    }
};

FormEditAccount::FormEditAccount(KLib::Account *_account, QWidget* _parent) :
    CAMSEGDialog(new QTabWidget(), CAMSEGDialog::DialogWithoutPicture, CAMSEGDialog::OkCancelButtons, _parent),
    m_account(_account)
{
    if (_account)
    {
        setWindowTitle(tr("Edit Account"));
    }
    else
    {
        setWindowTitle(tr("Add Account"));
    }

    // UI
    m_tabs = qobject_cast<QTabWidget*>(centralWidget());
    m_tabs->setTabPosition(QTabWidget::West);
    m_tabs->tabBar()->setStyle(new CustomTabStyle());

    //Main Tab
    m_mainTab           = new QWidget();
    m_pictureSelector   = new PictureSelector(this);
    m_txtName           = new QLineEdit(this);
    m_txtCode           = new QLineEdit(this);
    m_chkPlaceholder    = new QCheckBox(this);
    m_cboCurrency       = new QComboBox(this);
    m_cboSecurity       = new QComboBox(this);
    m_cboInstitution    = new QComboBox(this);
    m_cboParent         = new AccountSelector(Flag_Placeholders, this);
    m_cboType           = new QComboBox(this);
    m_txtNote           = new QTextEdit(this);
    m_lblCreditLimit    = new QLabel(tr("Credit &Limit:"), this);
    m_txtCreditLimit    = new AmountEdit(this);

    m_lblCreditLimit->setBuddy(m_txtCreditLimit);

    m_timer = new QTimer(this);

    m_cboParent->setMinimumWidth(250);
    m_cboCurrency->setMinimumWidth(150);
    m_cboSecurity->setMinimumWidth(150);
    m_cboInstitution->setMinimumWidth(150);
    m_cboType->setMinimumWidth(150);

    setPictureLabel(m_pictureSelector);

    m_btnAddSecurity = new QPushButton(Core::icon("list-add"), "", this);
    m_btnAddSecurity->setMaximumWidth(24);

    m_btnAddInstitution = new QPushButton(Core::icon("list-add"), "", this);
    m_btnAddInstitution->setMaximumWidth(24);

    QPushButton* btnSelectPicture = new QPushButton(Core::icon("picture"), tr("Pictu&re"), this);
    addButton(btnSelectPicture, 0, AtLeft);

    QHBoxLayout* secLayout = new QHBoxLayout();
    secLayout->addWidget(m_cboSecurity);
    secLayout->addWidget(m_btnAddSecurity);

    QHBoxLayout* instLayout = new QHBoxLayout();
    instLayout->addWidget(m_cboInstitution);
    instLayout->addWidget(m_btnAddInstitution);

    QFormLayout* layout = new QFormLayout(m_mainTab);
    layout->addRow(tr("&Name:"), m_txtName);
    layout->addRow(tr("&Code:"), m_txtCode);
    layout->addRow(tr("Place&holder?"), m_chkPlaceholder);
    layout->addRow(tr("C&urrency:"), m_cboCurrency);
    layout->addRow(tr("&Security:"), secLayout);
    layout->addRow(tr("&Institution:"), instLayout);
    layout->addRow(tr("&Parent:"), m_cboParent);
    layout->addRow(tr("&Type:"), m_cboType);
    layout->addRow(m_lblCreditLimit, m_txtCreditLimit);

    static_cast<QLabel*>(layout->labelForField(secLayout))->setBuddy(m_cboSecurity);
    static_cast<QLabel*>(layout->labelForField(instLayout))->setBuddy(m_cboInstitution);

    //setCentralWidget(m_tabs);
    m_tabs->addTab(m_mainTab, Core::icon("bank-account"), tr("General"));

    //Extra tabs
    for (fn_buildTab f : m_registeredTabs)
    {
        AccountEditTab* tab = f(this);
        m_extraTabs << tab;
        m_tabs->addTab(tab, Core::icon(tab->tabIcon()), tab->tabName());
    }

    //Note tab
    m_tabs->addTab(m_txtNote, Core::icon("note"), tr("Note"));

    // Fill widgets
    m_cboSecurity->setModel(SecurityController::sortProxy(this));
    m_cboSecurity->setModelColumn(SecurityColumn::NAME);

    m_cboCurrency->setModel(CurrencyController::sortProxy(this));
    m_cboCurrency->setModelColumn(1);
    m_cboInstitution->setModel(InstitutionController::sortProxy(this));

    connect(m_cboParent, SIGNAL(currentAccountChanged(KLib::Account*)), this, SLOT(onParentAccountChanged(KLib::Account*)));
    connect(m_cboType,   SIGNAL(currentIndexChanged(int)),              this, SLOT(onAccountTypeChanged()));

    connect(m_btnAddSecurity, &QPushButton::clicked, this, &FormEditAccount::addSecurity);
    connect(m_btnAddInstitution, &QPushButton::clicked, this, &FormEditAccount::addInstitution);
    connect(btnSelectPicture, &QPushButton::clicked, m_pictureSelector, &PictureSelector::selectPicture);

    connect(m_txtName, SIGNAL(textChanged(QString)), this, SLOT(updateTitle(QString)));


    if (m_account)
    {
        if (m_account->type() == AccountType::INVESTMENT)
        {
            m_cboSecurity->setEnabled(true);
            m_btnAddSecurity->setEnabled(true);
            m_cboCurrency->setEnabled(false);
        }
        else
        {
            m_cboSecurity->setEnabled(false);
            m_btnAddSecurity->setEnabled(false);
            m_cboCurrency->setEnabled(true);
        }

        fillData();

        if (   m_account->type() == AccountType::TOPLEVEL
            || m_account->type() == AccountType::TRADING
            || (m_account->ledger() && m_account->ledger()->count()))
        {
            m_chkPlaceholder->setEnabled(false);
        }

        updateTitle(m_account->name());
    }
    else
    {
        onParentAccountChanged(m_cboParent->currentAccount());
        onAccountTypeChanged();

        m_cboInstitution->setCurrentIndex(m_cboInstitution->count()-1);
        m_cboCurrency->setCurrentIndex(m_cboCurrency->findData(Account::getTopLevel()->mainCurrency()));
        updateTitle("");
    }

    m_txtName->setFocus();
    showCreditLimit();
}

void FormEditAccount::updateTitle(const QString& _title)
{
    if (_title.isEmpty())
    {
        setTitle(windowTitle());
    }
    else
    {
        setTitle(_title);
    }
}

void FormEditAccount::setParentAccount(KLib::Account* _parent)
{
    if (_parent)
    {
        m_cboParent->setCurrentIndex(m_cboParent->findData(_parent->id()));
    }
}

QSize FormEditAccount::sizeHint() const
{
    return QSize(700, 500);
}

void FormEditAccount::registerTab(fn_buildTab _buildTab)
{
    m_registeredTabs << _buildTab;
}

void FormEditAccount::showCreditLimit()
{
    bool show = m_cboType->currentData().toInt() == AccountType::CREDITCARD;

    m_lblCreditLimit->setVisible(show);
    m_txtCreditLimit->setVisible(show);
}

void FormEditAccount::fillData()
{
    m_txtName->setText(m_account->name());
    m_txtCode->setText(m_account->code());
    m_chkPlaceholder->setChecked(m_account->isPlaceholder());
    m_cboCurrency->setCurrentIndex(m_cboCurrency->findData(m_account->mainCurrency()));
    m_cboSecurity->setCurrentIndex(m_cboSecurity->findData(m_account->idSecurity()));
    m_cboInstitution->setCurrentIndex(m_cboInstitution->findData(m_account->idInstitution()));
    m_pictureSelector->setIdPicture(m_account->idPicture());
    m_pictureSelector->setDefaultPicture(Core::pixmap(Account::typeToIcon(m_account->type())));

    m_cboParent->setCurrentIndex(m_cboParent->findData(m_account->parent()->id()));
    onParentAccountChanged(m_cboParent->currentAccount());

    m_cboType->setCurrentIndex(m_cboType->findData(m_account->type()));
    m_txtNote->setPlainText(m_account->note());

    if (!m_account->isPlaceholder() && m_account->ledger()->count())
    {
        m_cboSecurity->setEnabled(false);
        //m_cboCurrency->setEnabled(false);
    }    

    if (m_account->type() == AccountType::CREDITCARD)
    {
        m_txtCreditLimit->setAmount(Amount::fromQVariant(m_account->properties()->get("creditlimit")));
    }

    for (AccountEditTab* t : m_extraTabs)
    {
        t->fillData(m_account);
    }

}

void FormEditAccount::onParentAccountChanged(KLib::Account* _account)
{
    QVariant type = m_cboType->currentData();
    m_cboType->clear();

    if (!_account)
    {
        return;
    }

    QList<int> types = Account::possibleTypes(_account->type());

    for (int t : types)
    {
        m_cboType->addItem(Core::icon(Account::typeToIcon(t)), Account::typeToString(t), t);
    }

    if (m_cboType->findData(type) != -1)
    {
        m_cboType->setCurrentIndex(m_cboType->findData(type));
    }

    for (AccountEditTab* t : m_extraTabs)
    {
        t->onParentAccountChanged();
    }
}

void FormEditAccount::onAccountTypeChanged()
{
    if (m_cboType->currentData().toInt() == AccountType::INVESTMENT)
    {
        m_cboSecurity->setEnabled(true);
        m_btnAddSecurity->setEnabled(true);
        m_cboCurrency->setEnabled(false);
    }
    else
    {
        m_cboSecurity->setEnabled(false);
        m_btnAddSecurity->setEnabled(false);
        m_cboCurrency->setEnabled(true);
    }

    if (m_account && !m_account->isPlaceholder() && m_account->ledger()->count())
    {
        m_cboSecurity->setEnabled(false);
        //m_cboCurrency->setEnabled(false);
    }

    if (m_cboType->currentIndex() != -1)
    {
        m_pictureSelector->setDefaultPicture(Core::pixmap(Account::typeToIcon(m_cboType->currentData().toInt())));
    }
    else
    {
        m_pictureSelector->setDefaultPicture(Core::pixmap("bank-account"));
    }

    showCreditLimit();

    for (AccountEditTab* t : m_extraTabs)
    {
        t->onAccountTypeChanged();
    }

    for (int i = 1; i < m_tabs->count()-1; ++i) //Exclude 1st (Main) and last (Note)
    {
        AccountEditTab* t = qobject_cast<AccountEditTab*>(m_tabs->widget(i));

        if (t)
        {
            m_tabs->setTabEnabled(i, t->enableIf(m_cboType->currentData().toInt()));
        }
    }
}

void FormEditAccount::addSecurity()
{
    /*
     * We need a timer since m_cboSecurity uses a proxy model to sort the SecurityController.
     * Because of that, the sort is done after setting the new index, which results in problems.
     * The timer bypasses that, setting the index after the sort is done.
     */

    FormEditSecurity* form = new FormEditSecurity(FormEditSecurity::SecurityClass::Security, nullptr, this);
    int idSec = 0;
    if (form->exec() == QDialog::Accepted)
    {
        idSec = form->addedSecurityId();
        m_timer->setInterval(500);

        connect(m_timer, &QTimer::timeout, [=]()
        {
            m_cboSecurity->setCurrentIndex(m_cboSecurity->findData(idSec));
            m_timer->stop();
            m_timer->disconnect();
        });
        m_timer->start();
    }

    delete form;
}

void FormEditAccount::addInstitution()
{
    /*
     * We need a timer since m_cboInstitution uses a proxy model to sort the InstitutionController.
     * Because of that, the sort is done after setting the new index, which results in problems.
     * The timer bypasses that, setting the index after the sort is done.
     */

    FormEditInstitutionPayee* form = new FormEditInstitutionPayee(FormEditInstitutionPayee::Edit_Institution, this);
    int idInst = 0;
    if (form->exec() == QDialog::Accepted)
    {
        idInst = form->addedId();
        m_timer->setInterval(500);

        connect(m_timer, &QTimer::timeout, [=]()
        {
            m_cboInstitution->setCurrentIndex(m_cboInstitution->findData(idInst));
            m_timer->stop();
            m_timer->disconnect();
        });
        m_timer->start();
    }

    delete form;
}

void FormEditAccount::accept()
{
    QStringList errors;
    // Validate
    if (m_txtName->text().isEmpty())
    {
        errors << tr("The account name is empty.");
    }

    if (m_cboType->currentData().toInt() != AccountType::INVESTMENT && m_cboCurrency->currentIndex() == -1)
    {
        errors << tr("The currency is invalid.");
    }

    if (m_cboType->currentData().toInt() == AccountType::INVESTMENT && m_cboSecurity->currentIndex() == -1)
    {
        errors << tr("The security is invalid.");
    }

    if (!m_cboParent->currentAccount())
    {
        errors << tr("The parent account is invalid.");
    }

    if (m_cboType->currentIndex() == -1)
    {
        errors << tr("The type is invalid.");
    }

    for (AccountEditTab* t : m_extraTabs)
    {
        if (t->enableIf(m_cboType->currentData().toInt()))
        {
            errors << t->validate();
        }
    }

    if (!errors.isEmpty())
    {
        QString strErrors;
        for (QString s : errors)
        {
            strErrors += "\t" + s + "\n";
        }
        QMessageBox::information(this,
                                 tr("Save Changes"),
                                 tr("The following errors prevent the account to be "
                                    "saved:\n %1").arg(strErrors),
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    try
    {
        int idInstitution = m_cboInstitution->currentIndex() == -1 ? -1
                                                                   : m_cboInstitution->currentData().toInt();
        if (m_account)
        {            
            m_account->holdToModify();

            // Check if the parent was changed
            if (m_account->parent() != m_cboParent->currentAccount() &&
                m_account->type() != m_cboType->currentData().toInt())
            {
                m_account->moveToParent(m_cboParent->currentAccount(),
                                        m_cboType->currentData().toInt());
            }
            else if (m_account->parent() != m_cboParent->currentAccount())
            {
                m_account->moveToParent(m_cboParent->currentAccount());
            }

            m_account->setType(m_cboType->currentData().toInt());

            if (m_chkPlaceholder->isEnabled())
                m_account->setIsPlaceholder(m_chkPlaceholder->isChecked());

            m_account->setName(m_txtName->text());
            if (m_cboSecurity->isEnabled()) m_account->setIdSecurity(m_cboSecurity->currentData().toInt());
            m_account->setIdInstitution(idInstitution);
            m_account->setCode(m_txtCode->text());
            m_account->setNote(m_txtNote->toPlainText());
        }
        else
        {
            m_account = m_cboParent->currentAccount()->addChild(m_txtName->text(),
                                                    m_cboType->currentData().toInt(),
                                                    m_cboCurrency->isEnabled() ? m_cboCurrency->currentData().toString() : "",
                                                    m_cboSecurity->isEnabled() ? m_cboSecurity->currentData().toInt() : Constants::NO_ID,
                                                    m_chkPlaceholder->isChecked(),
                                                    idInstitution,
                                                    m_txtCode->text(),
                                                    m_txtNote->toPlainText());

            m_account->holdToModify();
        }

        m_account->setIdPicture(m_pictureSelector->idPicture());

        if (m_account->type() == AccountType::CREDITCARD
            && m_txtCreditLimit->amount() > 0)
        {
            m_account->properties()->set("creditlimit", m_txtCreditLimit->amount().toQVariant());
        }
        else
        {
            m_account->properties()->remove("creditlimit");
        }

        for (AccountEditTab* t : m_extraTabs)
        {
            if (t->enableIf(m_account->type()))
            {
                t->save(m_account);
            }
        }

        //Have to set the main currency at the end since may have changed the secondary currencies...
        if (m_cboCurrency->isEnabled()) m_account->setCurrency(m_cboCurrency->currentData().toString());

        m_account->doneHoldToModify();

        done(QDialog::Accepted);
    }
    catch (ModelException e)
    {
        if (m_account) m_account->doneHoldToModify();
        QMessageBox::warning(this,
                             tr("Save Changes"),
                             tr("An error has occured while saving the account:\n\n%1").arg(e.description()));
    }
}



