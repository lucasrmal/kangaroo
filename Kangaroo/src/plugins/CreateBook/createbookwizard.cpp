#include "createbookwizard.h"
#include <KangarooLib/controller/io.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/modelexception.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QDebug>
#include <QMessageBox>

using namespace KLib;

QWizardPage* createIntroPage()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle("Introduction");
    page->setPixmap(QWizard::WatermarkPixmap, Core::pixmap("bills-watermark"));

    QLabel *label = new QLabel("This wizard will help you create a new Kangaroo Book.");
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

ConclusionPage::ConclusionPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Create the book");
    setPixmap(QWizard::WatermarkPixmap, Core::pixmap("bills-watermark"));

    QLabel *label = new QLabel("The wizard has now assembled all the information required "
                               "to create the new book. Click on Fininsh to create it.");
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    setLayout(layout);
}

bool ConclusionPage::validatePage()
{
    try
    {
        //Create the new book
        Core::instance()->loadNew();

        //Set the name
        IO::instance()->setName(field("BookName").toString());

        //Add the currencies
        QList<CurrencyManager::WorldCurrency> list = static_cast<CreateBookWizard*>(wizard())->currencies();
        bool hasDefault = false;

        for (CurrencyManager::WorldCurrency c : list)
        {
            if (c.code == Constants::DEFAULT_CURRENCY_CODE) hasDefault = true;

            if (!CurrencyManager::instance()->has(c.code))
            {
                CurrencyManager::instance()->add(c.code, c.name, c.symbol, c.precision);
            }
            else
            {
                Currency* cur = CurrencyManager::instance()->get(c.code);
                cur->setName(c.name);
                cur->setCustomSymbol(c.symbol);
                cur->setPrecision(c.precision);
            }
        }

        //Set the default currency
        QString defCur = field("BookCurrency").toString();
        Account::getTopLevel()->setCurrency(defCur);

        if (!hasDefault)
        {
            CurrencyManager::instance()->remove(Constants::DEFAULT_CURRENCY_CODE);
        }

        //Add the base accounts

        if (!hasDefault)
        Account::getTopLevel()->addChild(tr("Assets"),
                                         AccountType::ASSET,
                                         defCur,
                                         Constants::NO_ID,
                                         true);

        Account::getTopLevel()->addChild(tr("Liabilities"),
                                         AccountType::LIABILITY,
                                         defCur,
                                         Constants::NO_ID,
                                         true);

        Account::getTopLevel()->addChild(tr("Equity"),
                                         AccountType::EQUITY,
                                         defCur,
                                         Constants::NO_ID,
                                         true);

        Account::getTopLevel()->addChild(tr("Expenses"),
                                         AccountType::EXPENSE,
                                         defCur,
                                         Constants::NO_ID,
                                         true);

        Account::getTopLevel()->addChild(tr("Income"),
                                         AccountType::INCOME,
                                         defCur,
                                         Constants::NO_ID,
                                         true);

        return true;
    }
    catch (ModelException e)
    {
        QMessageBox::warning(this,
                             tr("Error"),
                             tr("An error has occured while creating the book: \n\n%1").arg(e.description()));
        Core::instance()->unloadFile();
        return false;
    }
}

SettingsPage::SettingsPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Book Settings"));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark.png"));

    QLabel* topLabel = new QLabel(tr("Please enter the name of the book and the "
                             "default currency. Note that these settings "
                             "can be changed later."));
    topLabel->setWordWrap(true);

    m_lblName = new QLineEdit(this);
    m_cboCurrency = new QComboBox(this);

    for (CurrencyManager::WorldCurrency c : CurrencyManager::worldCurrencies())
    {
        m_cboCurrency->addItem(c.name, c.code);
    }

    QFormLayout *formlayout = new QFormLayout;
    formlayout->addRow(tr("Book &Name:"), m_lblName);
    formlayout->addRow(tr("Default &Currency:"), m_cboCurrency);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(topLabel);
    layout->addSpacing(10);
    layout->addLayout(formlayout);
    setLayout(layout);

    registerField("BookName*", m_lblName);
    registerField("BookCurrency", m_cboCurrency);
}

CurrenciesPage::CurrenciesPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Currencies"));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark.png"));

    QLabel* topLabel = new QLabel(tr("Please select the currencies you wish to "
                                     "work with in the book. More currencies can "
                                     "be added later."));
    topLabel->setWordWrap(true);

    m_listCurrencies = new QListWidget(this);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(topLabel);
    layout->addSpacing(10);
    layout->addWidget(m_listCurrencies);
    setLayout(layout);
}

void CurrenciesPage::initializePage()
{
    m_listCurrencies->clear();

    for (CurrencyManager::WorldCurrency c : CurrencyManager::worldCurrencies())
    {
        QListWidgetItem* i = new QListWidgetItem(c.name, m_listCurrencies);
        i->setData(Qt::UserRole, c.code);
        i->setFlags(i->flags() | Qt::ItemIsUserCheckable);
        i->setCheckState(Qt::Unchecked);

        if (c.code == field("BookCurrency").toString())
        {
            i->setCheckState(Qt::Checked);
            i->setFlags(0);
        }
    }

}

CreateBookWizard::CreateBookWizard(QWidget *parent) :
    QWizard(parent)
{
    setWizardStyle(QWizard::ModernStyle);
    setWindowTitle(tr("Create New Book"));

    setDefaultProperty("QComboBox", "currentData", "currentIndexChanged()");

    for (CurrencyManager::WorldCurrency c : CurrencyManager::worldCurrencies())
    {
        m_currencies[c.code] = c;
    }

    m_currencyPage = new CurrenciesPage(this);

    addPage(createIntroPage());
    addPage(new SettingsPage(this));
    addPage(m_currencyPage);
    addPage(new ConclusionPage(this));
}

QList<CurrencyManager::WorldCurrency> CreateBookWizard::currencies() const
{
    QList<CurrencyManager::WorldCurrency> list;

    for (int i = 0; i < m_currencyPage->m_listCurrencies->count(); ++i)
    {
        if (m_currencyPage->m_listCurrencies->item(i)->checkState() == Qt::Checked)
        {
            list << m_currencies[m_currencyPage->m_listCurrencies->item(i)->data(Qt::UserRole).toString()];
        }
    }

    return list;
}
