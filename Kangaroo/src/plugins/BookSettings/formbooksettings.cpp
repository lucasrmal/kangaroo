#include "formbooksettings.h"
#include "booksettingstab.h"

#include <QFormLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QSortFilterProxyModel>

#include <KangarooLib/ui/core.h>
#include <KangarooLib/controller/currencycontroller.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/controller/io.h>

using namespace KLib;

QList<BookSettingsTabFactorySuper*> FormBookSettings::m_registeredTabs = QList<BookSettingsTabFactorySuper*>();

FormBookSettings::FormBookSettings(QWidget *_parent) :
    CAMSEGDialog(DialogWithPicture, OkCancelButtons, _parent)
{
    setBothTitles(tr("Book Settings"));
    setPicture(Core::pixmap("modify"));


    m_tabs = new QTabWidget(this);

    //Main tab
    QWidget* mainTab = new QWidget(this);
    m_txtBookName = new QLineEdit(this);
    m_cboMainCurrency = new QComboBox(this);

    m_cboMainCurrency->setModel(CurrencyController::sortProxy(this));
    m_cboMainCurrency->setModelColumn(1);

    QFormLayout* mainTabLayout = new QFormLayout(mainTab);
    mainTabLayout->addRow(tr("Book &Name:"), m_txtBookName);
    mainTabLayout->addRow(tr("Main &Currency:"), m_cboMainCurrency);

    //Setup tabs
    setCentralWidget(m_tabs);
    m_tabs->addTab(mainTab, tr("General"));

    for (BookSettingsTabFactorySuper* sup : m_registeredTabs)
    {
        BookSettingsTab* tab = sup->buildTab(this);
        m_extraTabs << tab;
        m_tabs->addTab(tab, Core::icon(tab->tabIcon()), tab->tabName());
    }

    //Load data
    m_txtBookName->setText(IO::instance()->currentName());
    m_cboMainCurrency->setCurrentIndex(m_cboMainCurrency->findData(Account::getTopLevel()->mainCurrency()));

    for (BookSettingsTab* tab : m_extraTabs)
    {
        tab->fillData();
    }
}

void FormBookSettings::accept()
{
    //Validate user data
    QStringList errors;

    if (m_txtBookName->text().isEmpty())
    {
        errors << tr("The book name is empty.");
    }

    if (m_cboMainCurrency->currentIndex() == -1)
    {
        errors << tr("The main currency is invalid.");
    }

    for (BookSettingsTab* t : m_extraTabs)
    {
        errors << t->validate();
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
                                 tr("The following errors prevent the settings to be "
                                    "saved:\n %1").arg(strErrors),
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    //Save changes
    try
    {
        IO::instance()->setName(m_txtBookName->text());
        Account::getTopLevel()->setCurrency(m_cboMainCurrency->currentData().toString());

        for (BookSettingsTab* t : m_extraTabs)
        {
            t->save();
        }

    }
    catch (ModelException e)
    {
        QMessageBox::warning(this,
                             tr("Save Changes"),
                             tr("An error has occured while saving the changes:\n\n%1").arg(e.description()));
    }

    //Accept!
    done(QDialog::Accepted);
}

void FormBookSettings::registerTab(BookSettingsTabFactorySuper* _tab)
{
    if (_tab)
    {
        m_registeredTabs << _tab;
    }
}

