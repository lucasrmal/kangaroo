#include "managerwidget.h"
#include "formeditsecurity.h"
#include "formeditcurrency.h"
#include "formmerge.h"
#include "formaddrate.h"
#include "formeditinstitutionpayee.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/controller/currencycontroller.h>
#include <KangarooLib/controller/institutioncontroller.h>
#include <KangarooLib/controller/payeecontroller.h>
#include <KangarooLib/controller/securitycontroller.h>
#include <KangarooLib/controller/indexcontroller.h>
#include <KangarooLib/controller/pricecontroller.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/payee.h>
#include <KangarooLib/model/institution.h>
#include <KangarooLib/model/security.h>
#include <KangarooLib/model/pricemanager.h>
#include <KangarooLib/model/modelexception.h>
#include <QPushButton>
#include <QInputDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QAction>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QToolButton>

using namespace KLib;

BetterTableView::BetterTableView(QWidget* _parent) :
    QTableView(_parent)
{

}

QList<int> BetterTableView::selectedRows() const
{
    QSet<int> uniqueRows;

    for (QModelIndex i : selectedIndexes())
    {
        uniqueRows.insert(i.row());
    }

    return uniqueRows.toList();
}

void BetterTableView::selectionChanged(const QItemSelection& _selected, const QItemSelection& _deselected)
{
    QTableView::selectionChanged(_selected, _deselected);

    //Emit the row changed signal
    emit selectedRowsChanged(selectedRows());
}

ManagerWidget::ManagerWidget(ManageType _type, QWidget* _parent) :
    QWidget(_parent),
    m_type(_type),
    m_sortFilterProxyModel(nullptr),
    m_tableModel(nullptr),
    m_findBar(nullptr)
{
    QLabel* lblPicture = new QLabel(this);
    QLabel* lblTitle = new QLabel(this);

    if (m_type != ManageType::Prices)
    {
        m_tableView = new BetterTableView(this);
    }
    else
    {
        m_treeView = new QTreeView(this);
    }
    QToolBar* bottomToolbar = new QToolBar(this);

    lblPicture->setFixedSize(48,48);

    m_layout = new QVBoxLayout(this);
    QHBoxLayout* layoutTop = new QHBoxLayout();

    m_layout->setContentsMargins(0,0,0,0);
    layoutTop->setContentsMargins(6,6,6,0);

    layoutTop->addWidget(lblPicture);
    layoutTop->addWidget(lblTitle);

    m_layout->addLayout(layoutTop);

    if (m_type != ManageType::Prices)
    {
        m_layout->addWidget(m_tableView);
    }
    else
    {
        m_layout->addWidget(m_treeView);
    }

    m_layout->addWidget(bottomToolbar);

    QFont f;
    f.setPointSize(14);
    f.setBold(true);
    lblTitle->setFont(f);


    bottomToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    bottomToolbar->setIconSize(QSize(24,24));

    //Spacer for the toolbar
    QWidget* sep1 = new QWidget();
    sep1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

    m_actAdd = bottomToolbar->addAction(Core::icon("new-row"), tr("New"),    this, SLOT(add()));

    if (m_type != ManageType::Prices)
    {
        m_actEdit = bottomToolbar->addAction(Core::icon("edit"),    tr("Edit"),   this, SLOT(edit()));
    }

    if (m_type == ManageType::Institutions || m_type == ManageType::Payees)
    {
        m_actMerge = bottomToolbar->addAction(Core::icon("merge"),   tr("Merge"),   this, SLOT(merge()));
        m_actMerge->setEnabled(false);
    }
    else
    {
        m_actMerge = nullptr;
    }

    m_actDelete = bottomToolbar->addAction(Core::icon("trash"), tr("Delete"), this, SLOT(remove()));
    bottomToolbar->addWidget(sep1);
    m_actFind = bottomToolbar->addAction(Core::icon("find"), tr("Search"), this, SLOT(search()));
    m_actFind->setShortcut(QKeySequence::Find);

    //Deactivate edit and remove by default since no rows selected...
    m_actDelete->setEnabled(false);

    int largeCol = 0;

    if (m_type != ManageType::Prices)
    {
        m_sortFilterProxyModel = new QSortFilterProxyModel(this);

        m_sortFilterProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        m_tableView->setModel(m_sortFilterProxyModel);
        m_tableView->setSortingEnabled(true);
        m_tableView->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);

        m_actEdit->setEnabled(false);
    }

    if (_type == ManageType::Securities)
    {
        m_tableView->horizontalHeader()->setSortIndicator(SecurityColumn::NAME, Qt::AscendingOrder);
    }

    switch (_type)
    {
    case ManageType::Currencies:
        lblPicture->setPixmap(Core::pixmap("currency"));
        lblTitle->setText(tr("Currencies"));
        m_tableModel = new CurrencyController(this);
        m_sortFilterProxyModel->setSourceModel(m_tableModel);
        largeCol = CurrencyColumn::NAME;
        m_tableView->setColumnWidth(CurrencyColumn::CODE, 70);
        m_tableView->setColumnWidth(CurrencyColumn::SYMBOL, 70);
        break;

    case ManageType::Institutions:
        lblPicture->setPixmap(Core::pixmap("institution"));
        lblTitle->setText(tr("Institutions"));
        m_tableModel = new InstitutionController(InstitutionController::NoEmpty, this);
        m_sortFilterProxyModel->setSourceModel(m_tableModel);
        largeCol = InstitutionColumn::NAME;

        m_tableView->setColumnWidth(InstitutionColumn::COUNTRY, 100);
        m_tableView->setColumnWidth(InstitutionColumn::PHONE, 100);
        m_tableView->setColumnWidth(InstitutionColumn::FAX, 100);
        m_tableView->setColumnWidth(InstitutionColumn::CONTACT_NAME, 160);
        m_tableView->setColumnWidth(InstitutionColumn::WEBSITE, 160);
        m_tableView->setColumnWidth(InstitutionColumn::INSTITUTION_NUMBER, 140);
        m_tableView->setColumnWidth(InstitutionColumn::ROUTING_NUMBER, 140);

        break;

    case ManageType::Payees:
        lblPicture->setPixmap(Core::pixmap("payee"));
        lblTitle->setText(tr("Payees"));
        m_tableModel = new PayeeController(this);
        m_sortFilterProxyModel->setSourceModel(m_tableModel);
        largeCol = PayeeColumn::NAME;

        m_tableView->setColumnWidth(PayeeColumn::ADDRESS, 230);
        m_tableView->setColumnWidth(PayeeColumn::COUNTRY, 100);
        m_tableView->setColumnWidth(PayeeColumn::PHONE, 100);
        m_tableView->setColumnWidth(PayeeColumn::FAX, 100);
        m_tableView->setColumnWidth(PayeeColumn::CONTACT_NAME, 160);
        m_tableView->setColumnWidth(PayeeColumn::WEBSITE, 160);

        break;

    case ManageType::Securities:
        lblPicture->setPixmap(Core::pixmap("security"));
        lblTitle->setText(tr("Securities"));
        m_tableModel = new SecurityController(this);
        m_sortFilterProxyModel->setSourceModel(m_tableModel);
        largeCol = SecurityColumn::NAME;
        break;

    case ManageType::Indexes:
        lblPicture->setPixmap(Core::pixmap("index"));
        lblTitle->setText(tr("Indexes"));
        m_tableModel = new IndexController(this);
        m_sortFilterProxyModel->setSourceModel(m_tableModel);
        largeCol = IndexColumn::NAME;
        break;

    case ManageType::Prices:

        lblPicture->setPixmap(Core::pixmap("price"));
        lblTitle->setText(tr("Rates and Prices"));

        m_treeView->setModel(new PriceController(this));
        m_treeView->setColumnWidth(PriceColumn::PAIR, 300);
        m_treeView->setColumnWidth(PriceColumn::DATE, 200);

        m_treeView->expandToDepth(0);
        break;

    default:
        return;
    }

    if (m_type != ManageType::Prices)
    {
        m_tableView->horizontalHeader()->setSectionResizeMode(largeCol, QHeaderView::Stretch);
        m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

        m_tableView->verticalHeader()->hide();

        connect(m_tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(edit()));
        connect(m_tableView, SIGNAL(selectedRowsChanged(QList<int>)), this, SLOT(onSelectedRowsChanged(QList<int>)));
    }
    else
    {
        m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        connect(m_treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(onSelectionChangeTreeView()));
    }
}

void ManagerWidget::add()
{
    switch (m_type)
    {

    case ManageType::Institutions:
    {
        FormEditInstitutionPayee formIns(FormEditInstitutionPayee::Edit_Institution, this);
        formIns.exec();
        break;
    }
    case ManageType::Payees:
    {
        FormEditInstitutionPayee formPayee(FormEditInstitutionPayee::Edit_Payee, this);
        formPayee.exec();
        break;
    }
    case ManageType::Currencies:
    {
        FormEditCurrency frmCur(nullptr, this);
        frmCur.exec();
        break;
    }
    case ManageType::Securities:
    {
        FormEditSecurity frmSec(FormEditSecurity::SecurityClass::Security, nullptr, this);
        frmSec.exec();
        break;
    }
    case ManageType::Indexes:
    {
        FormEditSecurity frmSec(FormEditSecurity::SecurityClass::Index, nullptr, this);
        frmSec.exec();
        break;
    }
    case ManageType::Prices:
    {
        FormAddRate frmPrice(this);

        //Preselect the pair corresponding to the current index if it is a pair or a rate.
        QModelIndex index = m_treeView->currentIndex();

        if (index.isValid())
        {
            PriceItem* i = static_cast<PriceItem*>(index.internalPointer());

            if (i->itemType == PriceItem::Exchange)
            {
                frmPrice.selectPair(i->pair);
            }
            else if (i->itemType == PriceItem::Rate)
            {
                frmPrice.selectPair(i->parent->pair);
            }
        }

        frmPrice.exec();
        break;
    }

    default:
        return;
    }
}

void ManagerWidget::remove()
{
    QString title = tr("Confirm Delete"),
            noUndo = tr("<b>This action cannot be undone.</b>");
    try
    {
        if (m_type == ManageType::Prices)
        {
            QModelIndex index = m_treeView->currentIndex();

            if (index.isValid())
            {
                PriceItem* i = static_cast<PriceItem*>(index.internalPointer());

                if (!i)
                {
                    return;
                }

                auto from = [](ExchangePair* p)
                {
                    if (p->isSecurity())
                    {
                        return p->securityFrom()->name();
                    }
                    else
                    {
                        return CurrencyManager::instance()->get(p->from())->name();
                    }
                };

                auto to = [](ExchangePair* p)
                {
                    return CurrencyManager::instance()->get(p->to())->name();
                };

                if (i->itemType == PriceItem::Exchange)
                {

                    if (QMessageBox::question(this,
                                              title,
                                              tr("Delete all rates from <b>%1</b> to <b>%2</b>?")
                                                .arg(from(i->pair)).arg(to(i->pair))
                                              + "<br><br>" + noUndo,
                                              QMessageBox::Yes | QMessageBox::No,
                                              QMessageBox::No) == QMessageBox::Yes)
                    {
                        PriceManager::instance()->remove(i->pair->from(), i->pair->to());
                    }
                }
                else if (i->itemType == PriceItem::Rate)
                {
                    PriceItem* pair = i->parent;

                    if (QMessageBox::question(this,
                                              title,
                                              tr("Delete this rate: <b>%1</b> to <b>%2</b> on %3?")
                                                .arg(from(pair->pair))
                                                .arg(to(pair->pair))
                                                .arg(i->date.toString(Core::instance()->dateFormat()))
                                              + "<br><br>" + noUndo,
                                              QMessageBox::Yes | QMessageBox::No,
                                              QMessageBox::No) == QMessageBox::Yes)
                    {
                        PriceManager::instance()->get(pair->pair->from(), pair->pair->to())
                                ->remove(i->date);
                    }
                }
            }
        }
        else
        {
            QList<int> rows = m_tableView->selectedRows();

            if (!rows.count())
            {
                return;
            }

            QVariantList ids;

            for (int row : rows)
            {
                ids << m_tableView->model()->data(m_tableView->model()->index(row, 0), Qt::UserRole);
            }

            //Check if all the indices can be removed
            if (m_type == ManageType::Currencies)
            {
                for (QVariant id : ids)
                {
                    if (!CurrencyManager::instance()->get(id.toString())->canRemove())
                    {
                        QMessageBox::information(this,
                                                 tr("Delete Currency"),
                                                 tr("The currency %1 cannot be deleted: it is in use in at least one account.")
                                                    .arg(CurrencyManager::instance()->get(id.toString())->name()));
                        return;
                    }
                }
            }
            else if (m_type == ManageType::Securities)
            {
                for (QVariant id : ids)
                {
                    if (!SecurityManager::instance()->get(id.toInt())->canRemove())
                    {
                        QMessageBox::information(this,
                                                 tr("Delete Security"),
                                                 tr("The security %1 cannot be deleted: it is in use in at least one account.")
                                                    .arg(SecurityManager::instance()->get(id.toInt())->name()));
                        return;
                    }
                }
            }

            QString msg;

            switch (m_type)
            {
            case ManageType::Currencies:
                if (ids.count() == 1)
                {
                    msg = tr("Delete this currency: <b>%1?</b>").arg(CurrencyManager::instance()->get(ids[0].toString())->name());
                }
                else
                {
                    msg = tr("Delete these %1 currencies?").arg(rows.count());
                }
                break;

            case ManageType::Institutions:
                if (ids.count() == 1)
                {
                    msg = tr("Delete this institution: <b>%1</b>? <br><br>This action will result in all "
                             "accounts with this institution set to "
                             "<i>No Institution</i>.").arg(InstitutionManager::instance()->get(ids[0].toInt())->name());
                }
                else
                {
                    msg = tr("Delete these %1 institutions? <br><br>This action will result in all "
                             "accounts with these institutions set to "
                             "<i>No Institution</i>.").arg(rows.count());
                }

                break;

            case ManageType::Payees:
                if (ids.count() == 1)
                {
                    msg = tr("Delete this payee: <b>%1</b>? <br><br>This action will result in all "
                             "transactions with this payee set to "
                             "<i>No Payee</i>.").arg(PayeeManager::instance()->get(ids[0].toInt())->name());
                }
                else
                {
                    msg = tr("Delete these %1 payees? <br><br>This action will result in all "
                             "transactions with these payees set to "
                             "<i>No Payee</i>.").arg(rows.count());
                }

                break;

            case ManageType::Securities:
                if (ids.count() == 1)
                {
                    msg = tr("Delete this security: <b>%1</b>?").arg(SecurityManager::instance()->get(ids[0].toInt())->name());
                }
                else
                {
                    msg = tr("Delete these %1 securities?").arg(rows.count());
                }

                break;

            case ManageType::Indexes:
                if (ids.count() == 1)
                {
                    msg = tr("Delete this index: <b>%1</b>?").arg(SecurityManager::instance()->get(ids[0].toInt())->name());
                }
                else
                {
                    msg = tr("Delete these %1 indexes?").arg(rows.count());
                }

                break;

            default:
                return;
            }

            if (QMessageBox::question(this,
                                      title,
                                      msg + "<br><br>" + noUndo,
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No) == QMessageBox::Yes)
            {
                switch (m_type)
                {
                case ManageType::Currencies:
                    for (QVariant id : ids)
                    {
                        CurrencyManager::instance()->remove(id.toString());
                    }
                    break;

                case ManageType::Institutions:
                    for (QVariant id : ids)
                    {
                        InstitutionManager::instance()->remove(id.toInt());
                    }
                    break;

                case ManageType::Payees:
                    for (QVariant id : ids)
                    {
                        PayeeManager::instance()->remove(id.toInt());
                    }
                    break;

                case ManageType::Securities:
                case ManageType::Indexes:
                    for (QVariant id : ids)
                    {
                        SecurityManager::instance()->remove(id.toInt());
                    }
                    break;

                default:
                    return;
                }
            }
        }
    }
    catch (ModelException e)
    {
        QMessageBox::warning(this,
                             tr("Delete"),
                             tr("An error has occured while processing the request:\n\n%1").arg(e.description()));
    }
}

void ManagerWidget::edit()
{
    if (m_type != ManageType::Prices)
    {
        if (m_tableView->currentIndex().row() < 0)
        {
            return;
        }

        QVariant id = m_tableView->model()->data(m_tableView->currentIndex(), Qt::UserRole);

        switch (m_type)
        {
        case ManageType::Institutions:
        {
            FormEditInstitutionPayee formIns(InstitutionManager::instance()->get(id.toInt()), this);
            formIns.exec();
            break;
        }
        case ManageType::Payees:
        {
            FormEditInstitutionPayee formPayee(PayeeManager::instance()->get(id.toInt()), this);
            formPayee.exec();
            break;
        }
        case ManageType::Currencies:
        {
            FormEditCurrency frmCur(CurrencyManager::instance()->get(id.toString()), this);
            frmCur.exec();
            break;
        }
        case ManageType::Securities:
        {
            FormEditSecurity frmSec(FormEditSecurity::SecurityClass::Security, SecurityManager::instance()->get(id.toInt()), this);
            frmSec.exec();
            break;
        }
        case ManageType::Indexes:
        {
            FormEditSecurity frmSec(FormEditSecurity::SecurityClass::Index, SecurityManager::instance()->get(id.toInt()), this);
            frmSec.exec();
            break;
        }
        default:
            break;
        }
    }
}

void ManagerWidget::filterTo(const QString& _filter)
{
    if (!m_sortFilterProxyModel) {
        return;
    }

  m_sortFilterProxyModel->setFilterFixedString(_filter);
    m_tableView->scrollTo(m_tableModel->)

//    m_accountTree->expandAll();
//    onCurrentAccountChanged(m_accountTree->currentAccount());
//    m_accountTree->scrollTo(m_accountTree->currentIndex());
}

void ManagerWidget::search()
{
    if (m_findBar) {
        return;
    }
    m_findBar = new QWidget(this);

    QHBoxLayout* layout = new QHBoxLayout(m_findBar);
    layout->setContentsMargins(0,0,0,0);

    QLineEdit* txtSearch = new QLineEdit(m_findBar);
    QToolButton* btnClose = new QToolButton(m_findBar);

    btnClose->setAutoRaise(true);

    QAction* actClose = new QAction(Core::icon("close"), "", this);
    actClose->setShortcut(QKeySequence("Esc"));
    btnClose->setDefaultAction(actClose);

    layout->addWidget(btnClose);
    layout->addWidget(txtSearch);

    txtSearch->setFocus();

    connect(actClose, &QAction::triggered, this, &ManagerWidget::doneSearch);
    connect(txtSearch, &QLineEdit::textEdited, this, &ManagerWidget::search);

    m_layout->insertWidget(m_layout->count()-1, m_findBar);
}

void ManagerWidget::doneSearch()
{
    if (!m_findBar) {
        return;
    }
    m_layout->removeWidget(m_findBar);
    m_findBar->deleteLater();
    m_findBar = nullptr;
    filterTo("");
}

void ManagerWidget::merge()
{
    switch (m_type)
    {
    case ManageType::Institutions:
    case ManageType::Payees:
    {
        //Get the list of ids
        QList<int> rows = m_tableView->selectedRows();

        QSet<int> ids;

        for (int row : rows)
        {
            ids.insert(m_tableView->model()->data(m_tableView->model()->index(row, 0), Qt::UserRole).toInt());
        }

        if (ids.count() < 2)
            return;

        FormMerge frmMerge(ids, m_type, this);
        frmMerge.exec();
        break;
    }
    default:
        return;
    }
}

void ManagerWidget::onSelectedRowsChanged(QList<int> _rows)
{
    //Activate actions given selected rows...
    m_actEdit->setEnabled(_rows.count() == 1);
    m_actDelete->setEnabled(_rows.count());

    if (m_actMerge)
    {
        m_actMerge->setEnabled(_rows.count() > 1);
    }
}

void ManagerWidget::onSelectionChangeTreeView()
{
    QModelIndex index = m_treeView->currentIndex();
    bool enable = false;

    if (index.isValid())
    {
        PriceItem* i = static_cast<PriceItem*>(index.internalPointer());

        enable = i && (i->itemType == PriceItem::Exchange
                       || i->itemType == PriceItem::Rate);
    }

    m_actDelete->setEnabled(enable);
}

unsigned int qHash(ManageType _key, unsigned int seed)
{
    return qHash((int) _key, seed);
}





