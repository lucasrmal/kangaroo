#include "categoryvaluecharteditor.h"
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/properties.h>
#include <KangarooLib/ui/core.h>
#include "categoryvaluechart.h"

#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>

using namespace KLib;

CategoryValueChartEditor::CategoryValueChartEditor(CategoryValueType _type, QWidget* _parent) :
    CAMSEGDialog(DialogWithPicture, OkCancelButtons, _parent),
    m_model(new CategoryEditorModel(_type, this)),
    m_view(new QTreeView(this))
{
    setBothTitles(tr("%1 Chart Settings").arg(_type == CategoryValueType::Income ? tr("Income") : tr("Spending")));
    setPicture(Core::pixmap("configure"));

    QLabel* lblExplanations = new QLabel(tr("Specify the accounts for which the data "
                                            "needs to be displayed on the chart. Each"
                                            "account selected will correspond to its own data point"
                                            "on the chart, and will include all the account "
                                            "hierarchy under it. None of the accounts in this "
                                            "hierarchy will be displayed; they will be included "
                                            "under the highest ancestor that is selected."), this);

    lblExplanations->setWordWrap(true);

    m_view->setModel(m_model);
    m_view->expandAll();

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget());
    mainLayout->addWidget(lblExplanations);
    mainLayout->addWidget(m_view);
}

void CategoryValueChartEditor::accept()
{
    //Save everything
    m_model->saveAll();
    done(QDialog::Accepted);
}

/************************************ MODEL ************************************/

CategoryEditorModel::CategoryEditorModel(CategoryValueType _type, QObject* _parent) :
    AccountController(Account::getTopLevel(),
                      true,
                      _type == CategoryValueType::Income ? AccountTypeFlags::Flag_Income
                                                         : AccountTypeFlags::Flag_Expense,
                      _parent),
    m_type(_type)
{
    loadData();
}

QVariant CategoryEditorModel::data(const QModelIndex& _index, int _role) const
{
    if (_index.isValid() && _role == Qt::CheckStateRole)
    {
        Account* a = accountForIndex(_index);

        if (a && m_checked.contains(a))
        {
            return m_checked[a] ? Qt::Checked : Qt::Unchecked;
        }
        else
        {
            return Qt::Unchecked;
        }
    }
    else
    {
        return AccountController::data(_index, _role);
    }
}

Qt::ItemFlags CategoryEditorModel::flags(const QModelIndex& _index) const
{
    if (_index.isValid())
    {
        return AccountController::flags(_index) | Qt::ItemIsUserCheckable;
    }
    else
    {
        return AccountController::flags(_index);
    }
}

bool CategoryEditorModel::setData(const QModelIndex& _index, const QVariant& _value, int _role)
{
    if (_index.isValid() && _role == Qt::CheckStateRole)
    {
        Account* a = accountForIndex(_index);

        if (a)
        {
            if (m_checked.contains(a))
            {
                m_checked[a] = !m_checked[a];
            }
            else
            {
                m_checked[a] = true;
            }

            emit dataChanged(_index, _index);

            if (!m_checked[a])
            {
                checkAllChildren(true, a, _index);
            }
            else
            {
                uncheckAllAncestors(a, _index);
                checkAllChildren(false, a, _index);
            }

            return true;
        }
    }

    return false;
}

void CategoryEditorModel::checkAllChildren(bool _check, KLib::Account* _parent, const QModelIndex& _index)
{
    for (Account* c : _parent->getChildren())
    {
        m_checked[c] = _check;
    }

    emit dataChanged(index(0, 0, _index), index(rowCount(_index)-1, 0, _index));
}

void CategoryEditorModel::uncheckAllAncestors(KLib::Account* _child, const QModelIndex& _index)
{
    QModelIndex index = _index;

    while (_child->parent() && _child->parent() != Account::getTopLevel())
    {
        m_checked[_child->parent()] = false;

        index = index.parent();
        _child = _child->parent();

        emit dataChanged(index, index);
    }
}

void CategoryEditorModel::loadData()
{
    int t = (m_type == CategoryValueType::Income ? AccountType::INCOME
                                                 : AccountType::EXPENSE);

    for (Account* a : Account::getTopLevel()->accounts())
    {
        if (a->type() == t && a->properties()->contains(CategoryValueModel::CATEGORY_PROP_TAG))
        {
            m_checked[a] = a->properties()->get(CategoryValueModel::CATEGORY_PROP_TAG).toBool();
        }
        else if (a->type() == t
                 && a->parent()
                 && a->parent()->parent()
                 && !a->parent()->parent()->parent()) //3rd level from root
        {
            m_checked[a] = true;
        }
    }
}

void CategoryEditorModel::saveAll()
{
    for (auto i = m_checked.begin(); i != m_checked.end(); ++i)
    {
        i.key()->properties()->set(CategoryValueModel::CATEGORY_PROP_TAG, i.value());
    }
}
