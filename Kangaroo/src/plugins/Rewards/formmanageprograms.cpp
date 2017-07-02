#include "formmanageprograms.h"
#include "rewardsprogramcontroller.h"
#include "tiereditor.h"
#include <QTableView>
#include <QPushButton>
#include <QMessageBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QSortFilterProxyModel>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/controller/currencycontroller.h>
#include <KangarooLib/model/currency.h>
#include "../ManageOthers/formeditcurrency.h"

using namespace KLib;

FormManagePrograms::FormManagePrograms(QWidget* _parent) :
    CAMSEGDialog(DialogWithPicture, CloseButton, _parent)
{
    setBothTitles(tr("Rewards Programs"));
    setPicture(Core::pixmap("favorites"));

    m_tableView = new QTableView(this);
    m_tableView->setModel(new RewardsProgramController(this));
    m_tableView->setItemDelegate(new RewardsProgramDelegate(this));
    setCentralWidget(m_tableView);

    m_tableView->setColumnWidth(RewardsProgramColumn::CURRENCY, 150);
    m_tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_tableView->verticalHeader()->hide();

    QPushButton* btnAdd = new QPushButton(Core::icon("add"), tr("&Add"), this);
    QPushButton* btnRemove = new QPushButton(Core::icon("remove"), tr("&Remove"), this);
    QPushButton* btnEditTiers = new QPushButton(Core::icon("tiers"), tr("Edit &Tiers"), this);

    connect(btnAdd,         &QPushButton::clicked, this, &FormManagePrograms::add);
    connect(btnRemove,      &QPushButton::clicked, this, &FormManagePrograms::remove);
    connect(btnEditTiers,   &QPushButton::clicked, this, &FormManagePrograms::editTiers);

    addButton(btnAdd,       0, AtLeft);
    addButton(btnEditTiers, 1, AtLeft);
    addButton(btnRemove,    2, AtLeft);
}

void FormManagePrograms::add()
{
    FormAddProgram form(this);
    form.exec();
}

void FormManagePrograms::remove()
{
    if (m_tableView->currentIndex().row() < 0)
    {
        return;
    }

    int id = m_tableView->model()->data(m_tableView->currentIndex(), Qt::UserRole).toInt();

    const QString msgBoxTitle = tr("Remove Rewards Program");

    if (RewardsProgramManager::instance()->get(id)->isInUse())
    {
        QMessageBox::information(this,
                                 msgBoxTitle,
                                 tr("Cannot remove this rewards program: it is in use in at least one account"));
        return;
    }

    if (QMessageBox::question(this,
                              msgBoxTitle,
                              tr("Remove %1").arg(RewardsProgramManager::instance()->get(id)->name()),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::Yes)
    {
        try
        {
            RewardsProgramManager::instance()->remove(id);
        }
        catch (ModelException e)
        {
            QMessageBox::warning(this,
                                 msgBoxTitle,
                                 tr("An error has occured while processing the request:\n\n%1").arg(e.description()));
        }
    }
}

void FormManagePrograms::editTiers()
{
    if (m_tableView->currentIndex().row() < 0)
    {
        return;
    }

    RewardsProgram* p = RewardsProgramManager::instance()
                        ->get(m_tableView->model()
                              ->data(m_tableView->currentIndex(), Qt::UserRole).toInt());

    QList<RewardTier> tiers = p->tiers();
    FormTierEditor form(tiers, this);

    if (form.exec() == QDialog::Accepted)
    {
        int maxId = 0;

        for (RewardTier& t : tiers)
        {
            maxId = std::max(maxId, t.id);
        }

        //Set the tier IDs
        for (RewardTier& t : tiers)
        {
            if (t.id == Constants::NO_ID)
            {
                t.id = ++maxId;
            }
        }

        try
        {
            p->setTiers(tiers);
        }
        catch (ModelException e)
        {
            QMessageBox::warning(this,
                                 tr("Edit Tiers"),
                                 tr("An error has occured while processing the request:\n\n%1").arg(e.description()));
        }
    }

}

//////////////////////////////////////////////////////////////////////////////////

FormAddProgram::FormAddProgram(QWidget* _parent) :
    CAMSEGDialog(DialogWithPicture, OkCancelButtons, _parent)
{
    setBothTitles(tr("Add Rewards Program"));
    setPicture(Core::pixmap("tiers"));

    m_timer = new QTimer(this);
    m_txtName = new QLineEdit(this);
    m_cboCurrency = new QComboBox(this);

    QPushButton* btnAddCurrency = new QPushButton(Core::icon("list-add"), "", this);
    btnAddCurrency->setMaximumWidth(24);

    m_tierEditor = new TierEditor(m_tiers, this);

    m_cboCurrency->setModel(CurrencyController::sortProxy(this));
    m_cboCurrency->setModelColumn(CurrencyColumn::NAME);

    QHBoxLayout* curLayout = new QHBoxLayout();
    curLayout->addWidget(m_cboCurrency);
    curLayout->addWidget(btnAddCurrency);

    QLabel* lblCur = new QLabel(tr("&Currency:"), this);
    lblCur->setBuddy(m_cboCurrency);

    QFormLayout* layout = new QFormLayout(centralWidget());
    layout->addRow(tr("&Name:"), m_txtName);
    layout->addRow(lblCur, curLayout);
    layout->addRow(m_tierEditor);

    connect(btnAddCurrency, &QPushButton::clicked, this, &FormAddProgram::addCurrency);

    m_txtName->setFocus();
}

void FormAddProgram::addCurrency()
{
    FormEditCurrency form(NULL, this);
    QString cur;

    if (form.exec() == QDialog::Accepted)
    {
        cur = form.currency()->code();
        m_timer->setInterval(500);

        connect(m_timer, &QTimer::timeout, [=]()
        {
            m_cboCurrency->setCurrentIndex(m_cboCurrency->findData(cur));
            m_timer->stop();
            m_timer->disconnect();
        });
        m_timer->start();
    }
}

void FormAddProgram::accept()
{
    if (m_txtName->text().isEmpty())
    {
        QMessageBox::information(this,
                                 windowTitle(),
                                 tr("Enter a name for the rewards program."));
        m_txtName->setFocus();
        return;
    }

    if (m_cboCurrency->currentIndex() < 0)
    {
        QMessageBox::information(this,
                                 windowTitle(),
                                 tr("Select a currency."));
        m_cboCurrency->setFocus();
        return;
    }

    if (!m_tierEditor->validate())
    {
        return;
    }

    //Set the tier IDs
    int i = 0;
    for (RewardTier& t : m_tiers)
    {
        t.id = i;
        ++i;
    }

    //All is good, add the program
    RewardsProgramManager::instance()->add(m_txtName->text(),
                                           m_cboCurrency->currentData().toString(),
                                           m_tiers);

    done(QDialog::Accepted);
}

