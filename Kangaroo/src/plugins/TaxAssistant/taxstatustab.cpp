#include "taxstatustab.h"

#include <KangarooLib/model/account.h>
#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QFormLayout>

using namespace KLib;

const QString TaxStatusTab::TAX_ITEM_PROPERTY = "TaxLineItem";

TaxStatusTab::TaxStatusTab(QWidget* _parent) : AccountEditTab(_parent)
{
    m_grpTaxRelated = new QGroupBox(tr("Tax Related Account"), this);
    m_cboLineItem = new QComboBox(this);
    m_cboLineItem->setEditable(true);

    m_cboLineItem->setMinimumWidth(300);

    QLabel* lblComment = new QLabel(tr("*Tip: Separate the form number from the tax line item by a colon. This"
                                       "will result in tax line items from the same form to be grouped together"
                                       "in the tax report. For example, if the form is <i>Form 672-A</i> and the line"
                                       "item is <i>Other Income</i>, use the format: "
                                       "<br><center><i>Form 672-A : Other Income</i></center>"),
                                    this);

    lblComment->setWordWrap(true);

    m_grpTaxRelated->setCheckable(true);
    m_grpTaxRelated->setChecked(false);

    QFormLayout* taxLayout = new QFormLayout(m_grpTaxRelated);
    taxLayout->addRow(tr("&Tax Line Item:"), m_cboLineItem);
    taxLayout->addRow(lblComment);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_grpTaxRelated);
    mainLayout->addStretch(1);
}

bool TaxStatusTab::enableIf(int _accountType) const
{
    switch (_accountType)
    {
    case AccountType::INCOME:
    case AccountType::EXPENSE:
        return true;

    default:
        return false;
    }
}

void TaxStatusTab::fillData(const KLib::Account* _account)
{
    //Find the current tax line items
    QSet<QString> taxItems;

    for (const Account* a : Account::getTopLevel()->accounts())
    {
        if (enableIf(a->type()) && a->properties()->contains(TAX_ITEM_PROPERTY))
        {
            taxItems.insert(a->properties()->get(TAX_ITEM_PROPERTY).toString());
        }
    }

    //Convert to list and sort
    QStringList taxItemList = taxItems.toList();
    std::sort(taxItemList.begin(), taxItemList.end());

    //Insert into combo box
    m_cboLineItem->addItems(taxItemList);


    if (_account->properties()->contains(TAX_ITEM_PROPERTY))
    {
        m_grpTaxRelated->setChecked(true);
        m_cboLineItem->setCurrentIndex(m_cboLineItem->findText(_account->properties()->get(TAX_ITEM_PROPERTY).toString()));
    }
}

void TaxStatusTab::save(KLib::Account* _account) const
{
    if (enableIf(_account->type()) && m_grpTaxRelated->isChecked())
    {
        _account->properties()->set(TAX_ITEM_PROPERTY, m_cboLineItem->currentText());
    }
    else
    {
        _account->properties()->remove(TAX_ITEM_PROPERTY);
    }
}

QStringList TaxStatusTab::validate() const
{
    if (m_grpTaxRelated->isChecked() && m_cboLineItem->currentText().isEmpty())
    {
        return {tr("Please enter a tax line item for this account or unselect the \"Tax Related\" box.")};
    }
    else
    {
        return {};
    }
}
