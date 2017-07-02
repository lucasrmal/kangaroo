#include "pricestab.h"
#include "formaddrate.h"
#include <KangarooLib/controller/pricecontroller.h>
#include <KangarooLib/model/pricemanager.h>
#include <KangarooLib/ui/core.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>

using namespace KLib;

PricesTab::PricesTab(QWidget *parent) :
    QWidget(parent),
    m_treeView(new QTreeView(this)),
    m_controller(new PriceController(this))
{
    m_treeView->setModel(m_controller);

    m_btnAddRate = new QPushButton(Core::icon("list-add"), tr("Add Rate"), this);
    m_btnRemovePair = new QPushButton(Core::icon("remove"), tr("Remove Pair"), this);

    connect(m_btnAddRate, SIGNAL(clicked()), this, SLOT(addRate()));
    connect(m_btnRemovePair, SIGNAL(clicked()), this, SLOT(removePair()));

    QVBoxLayout* btnLayout = new QVBoxLayout();
    btnLayout->addWidget(m_btnAddRate);
    btnLayout->addWidget(m_btnRemovePair);
    btnLayout->addStretch(2);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_treeView);
    layout->addLayout(btnLayout);
    setLayout(layout);



}

void PricesTab::addRate()
{
    FormAddRate f;
    f.exec();
}

void PricesTab::removePair()
{
    QModelIndex index = m_treeView->currentIndex();

    if (index.isValid())
    {
        PriceItem* i = static_cast<PriceItem*>(index.internalPointer());

        if (i && i->itemType == PriceItem::Exchange)
        {
            if (QMessageBox::question(this,
                                      tr("Remove Exchange Pair"),
                                      tr("Remove this exchange pair from %1 to %2? All usages of this pair will be set to 0.")
                                      .arg(i->pair->from()).arg(i->pair->to()),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No) == QMessageBox::Yes)
            {
                PriceManager::instance()->remove(i->pair->from(), i->pair->to());
            }
        }
    }
}
