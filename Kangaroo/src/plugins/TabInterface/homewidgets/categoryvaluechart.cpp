#include "categoryvaluechart.h"
#include "categoryvaluecharteditor.h"

#include <KangarooLib/ui/widgets/percchart.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/properties.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>

#include <QVBoxLayout>
#include <QDate>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>

const QString CategoryValueModel::CATEGORY_PROP_TAG = "CategoryValue";

using namespace KLib;

CategoryValueChart::CategoryValueChart(CategoryValueType _type, QWidget* _parent) :
    IHomeWidget(new QWidget(), _parent),
    m_model(new CategoryValueModel(_type, this))
{
    m_chart = new PercChart(ChartStyle::Pie, 0, 1, Qt::EditRole, this);
    m_chart->setModel(m_model);

    QPushButton* btnConfigure = new QPushButton(Core::icon("configure"), "", this);
    m_cboBalanceSelector = new QComboBox(this);
    m_spinYear = new QSpinBox(this);
    m_cboMonth = new QComboBox(this);

    m_spinYear->setMinimum(1900);
    m_spinYear->setMaximum(9999);

    m_btnPrevious = new QPushButton(this);
    m_btnNext = new QPushButton(this);

    m_btnPrevious->setMaximumWidth(25);
    m_btnNext->setMaximumWidth(25);

    m_btnPrevious->setIcon(Core::icon("1leftarrow"));
    m_btnNext->setIcon(Core::icon("1rightarrow"));

    for (int i = 1; i <= 12; ++i)
        m_cboMonth->addItem(QDate::longMonthName(i));

    m_cboMonth->setCurrentIndex(QDate::currentDate().month()-1);

    m_cboBalanceSelector->addItem(tr("Through Today"));
    m_cboBalanceSelector->addItem(tr("All Time"));
    m_cboBalanceSelector->addItem(tr("Year"));
    m_cboBalanceSelector->addItem(tr("Month"));
    m_cboBalanceSelector->setMaximumWidth(200);

    QHBoxLayout* layoutBottom = new QHBoxLayout();
    layoutBottom->addWidget(m_cboBalanceSelector);
    layoutBottom->addWidget(m_spinYear);
    layoutBottom->addWidget(m_cboMonth);
    layoutBottom->addSpacing(3);
    layoutBottom->addWidget(m_btnPrevious);
    layoutBottom->addWidget(m_btnNext);
    layoutBottom->addStretch(10);
    layoutBottom->addWidget(btnConfigure);


    QVBoxLayout* layout = new QVBoxLayout(centralWidget());
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_chart);
    layout->addLayout(layoutBottom);

    m_chart->setMinimumHeight(420);

    //Set default dates
    m_cboBalanceSelector->setCurrentIndex((int) BalanceType::Month);
    m_spinYear->setValue(QDate::currentDate().year());
    m_cboMonth->setCurrentIndex(QDate::currentDate().month()-1);
    updateBalanceDates();

    connect(btnConfigure, &QPushButton::clicked, this, &CategoryValueChart::configure);

    connect(m_cboBalanceSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeBalanceType(int)));

    connect(m_spinYear, SIGNAL(valueChanged(int)),        SLOT(updateBalanceDates()));
    connect(m_cboMonth, SIGNAL(currentIndexChanged(int)), SLOT(updateBalanceDates()));
    connect(m_btnNext, &QPushButton::clicked, this, &CategoryValueChart::goNext);
    connect(m_btnPrevious, &QPushButton::clicked, this, &CategoryValueChart::goPrevious);
}

QString CategoryValueChart::titleFor(CategoryValueType _type)
{
    switch (_type)
    {
    case CategoryValueType::Income:
        return tr("Income Chart");

    case CategoryValueType::Spending:
        return tr("Spending Chart");

    default:
        return "";
    }
}

QString CategoryValueChart::title() const
{
    return titleFor(m_model->type());
}

void CategoryValueChart::configure()
{
    if (m_model->configure())
    {
        m_chart->resetModel();
    }
}

void CategoryValueChart::goPrevious()
{
  switch ((BalanceType) m_cboBalanceSelector->currentIndex())
  {
  case BalanceType::Year:
    m_spinYear->setValue(m_spinYear->value()-1);
    updateBalanceDates();
    break;

  case BalanceType::Month:
    if (m_cboMonth->currentIndex() > 0)
    {
      m_cboMonth->setCurrentIndex(m_cboMonth->currentIndex()-1);
    }
    else
    {
      m_spinYear->setValue(m_spinYear->value()-1);
      m_cboMonth->setCurrentIndex(11);
    }
    updateBalanceDates();
    break;

  default:
    break;
  }
}

void CategoryValueChart::goNext()
{
  switch ((BalanceType) m_cboBalanceSelector->currentIndex())
  {
  case BalanceType::Year:
    m_spinYear->setValue(m_spinYear->value()+1);
    updateBalanceDates();
    break;

  case BalanceType::Month:
    if (m_cboMonth->currentIndex() < 11)
    {
      m_cboMonth->setCurrentIndex(m_cboMonth->currentIndex()+1);
    }
    else
    {
      m_spinYear->setValue(m_spinYear->value()+1);
      m_cboMonth->setCurrentIndex(0);
    }
    updateBalanceDates();
    break;

  default:
    break;
  }
}

void CategoryValueChart::onChangeBalanceType(int _index)
{
    switch ((BalanceType) _index)
    {
    case BalanceType::ThroughToday:
    case BalanceType::AllTime:
        m_cboMonth->setEnabled(false);
        m_spinYear->setEnabled(false);
        m_btnPrevious->setEnabled(false);
        m_btnNext->setEnabled(false);
        break;

    case BalanceType::Year:
        m_cboMonth->setEnabled(false);
        m_spinYear->setEnabled(true);
        m_btnPrevious->setEnabled(true);
        m_btnNext->setEnabled(true);
        m_spinYear->setFocus();
        break;

    case BalanceType::Month:
        m_cboMonth->setEnabled(true);
        m_spinYear->setEnabled(true);
        m_btnPrevious->setEnabled(true);
        m_btnNext->setEnabled(true);
        m_cboMonth->setFocus();
        break;
    }

    updateBalanceDates();
}

void CategoryValueChart::updateBalanceDates()
{
    QDate begin, end;

    switch ((BalanceType) m_cboBalanceSelector->currentIndex())
    {
    case BalanceType::ThroughToday:
        end = QDate::currentDate();
        break;

    case BalanceType::Year:
        begin = QDate(m_spinYear->value(), 1, 1);
        end = QDate(m_spinYear->value(), 12, 31);
        break;

    case BalanceType::Month:
        begin = QDate(m_spinYear->value(), m_cboMonth->currentIndex()+1, 1);
        end = QDate(m_spinYear->value(), m_cboMonth->currentIndex()+1, begin.daysInMonth());
        break;

    case BalanceType::AllTime: //Nothing to do :-)
        break;
    }

    m_model->setBalancesBetween(begin, end);

}


CategoryValueModel::CategoryValueModel(CategoryValueType _type, QObject* _parent) :
    QAbstractTableModel(_parent),
    m_type(_type)
{
    QDate current = QDate::currentDate();
    m_startDate = QDate(current.year(), current.month(), 1);
    m_endDate   = QDate(current.year(), current.month(), current.daysInMonth());

    loadData();
}

void CategoryValueModel::loadData()
{
    auto isCovered = [] (Account* a) {

        while (a->parent())
        {
            a = a->parent();

            if (a->properties()->contains(CATEGORY_PROP_TAG)
                && a->properties()->get(CATEGORY_PROP_TAG).toBool())
            {
                return true;
            }
        }

        return false;
    };


    for (Account* a : Account::getTopLevel()->accounts())
    {
        bool add = false;

        if (isRightType(a))
        {
            if (a->properties()->contains(CategoryValueModel::CATEGORY_PROP_TAG))
            {
                if (!isCovered(a))
                {
                    add = a->properties()->get(CategoryValueModel::CATEGORY_PROP_TAG).toBool();
                }
            }
            else if (!isCovered(a)
                     && a->parent()
                     && a->parent()->parent()
                     && !a->parent()->parent()->parent()) //3rd level from root
            {
                add = true;
            }

            if (add)
            {
                m_accountIndex[a->id()] = m_accounts.size();
                m_accounts << a;
            }
        }
    }

    //By default, show 2nd level of everything
//    for (Account* a : Account::getTopLevel()->getChildren())
//    {
//        if (isRightType(a))
//        {
//            for (Account* category : a->getChildren())
//            {
//            }
//        }
//    }
}

bool CategoryValueModel::isRightType(KLib::Account* _a)
{
    return (_a->type() == AccountType::EXPENSE && m_type == CategoryValueType::Spending)
            || (_a->type() == AccountType::INCOME && m_type == CategoryValueType::Income);
}

int CategoryValueModel::rowCount(const QModelIndex&) const
{
    return m_accounts.count();
}

int CategoryValueModel::columnCount(const QModelIndex&) const
{
    return 2;
}

QVariant CategoryValueModel::data(const QModelIndex& _index, int _role) const
{
    if (!_index.isValid() || (_role != Qt::EditRole && _role != Qt::DisplayRole))
    {
        return QVariant();
    }

    switch (_index.column())
    {
    case 0:
        return m_accounts[_index.row()]->name();

    case 1:
    {
        Amount value = m_accounts[_index.row()]->treeValueBetween(m_startDate, m_endDate);
        return _role == Qt::DisplayRole ? QVariant(m_accounts[_index.row()]->formatAmount(value))
                                        : QVariant(value.toDouble());
    }
    }

    return QVariant();
}

QVariant CategoryValueModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    if (_orientation == Qt::Horizontal && _role == Qt::DisplayRole)
    {
        switch (_section)
        {
        case 0:
            return tr("Category");

        case 1:
            return tr("Total");
        }
    }

    return QVariant();
}

void CategoryValueModel::setBalancesBetween(const QDate& _start, const QDate& _end)
{
    if (m_startDate != _start || m_endDate != _end)
    {
        m_startDate = _start;
        m_endDate   = _end;

        emit dataChanged(index(0, 1), index(rowCount()-1, 1));
    }
}

bool CategoryValueModel::configure()
{
    CategoryValueChartEditor* editor = new CategoryValueChartEditor(m_type, Core::instance()->mainWindow());

    bool ok = editor->exec() == QDialog::Accepted;

    if (ok)
    {
        beginResetModel();
        m_accounts.clear();
        m_accountIndex.clear();
        loadData();
        endResetModel();
    }

    delete editor;
    return ok;
}

int CategoryValueModel::indexFor(KLib::Account* _a, bool* _inList)
{
    if (_inList) *_inList = false;

    for (int i = 0; i < m_accounts.count(); ++i)
    {
        if (m_accounts[i] == _a)
        {
            if (_inList) *_inList = true;
            return i;
        }
        else if (m_accounts[i]->isAncestorOf(_a))
        {
            return i;
        }
    }

    return -1;
}

void CategoryValueModel::onAccountAdded(KLib::Account* _a)
{
    if (isRightType(_a))
    {
        int idx = indexFor(_a);

        if (idx != -1) //This is a subchild, so we're good!
        {
            emit dataChanged(index(idx, 1), index(idx, 1));
        }
        else //This is a new top-level, so we add it to our list
        {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            m_accountIndex[_a->id()] = m_accounts.size();
            m_accounts.append(_a);
            _a->properties()->set(CATEGORY_PROP_TAG, true);
            endInsertRows();
        }
    }
}

void CategoryValueModel::onAccountRemoved(KLib::Account* _a)
{
    if (isRightType(_a))
    {
        bool inList;
        int idx = indexFor(_a, &inList);

        if (inList)
        {
            //Remove it
            beginRemoveRows(QModelIndex(), idx, idx);

            m_accounts.removeAt(idx);
            m_accountIndex.remove(_a->id());

            for (int i = idx; i < m_accounts.size(); ++i)
            {
                m_accountIndex[m_accounts[i]->id()] = i;
            }

            endRemoveRows();
        }
        else if (idx != -1)
        {
            emit dataChanged(index(idx, 1), index(idx, 1));
        }
    }
}

void CategoryValueModel::onAccountModified(KLib::Account* _a)
{
    if (isRightType(_a))
    {
        //Account hierarchy not modified here since using remove/added.
        int idx = indexFor(_a);

        if (idx != -1)
        {
            emit dataChanged(index(idx, 1), index(idx, 1));
        }
        else //This is a new top-level, so we add it to our list
        {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            _a->properties()->set(CATEGORY_PROP_TAG, true);
            m_accountIndex[_a->id()] = m_accounts.size();
            m_accounts.append(_a);
            endInsertRows();
        }
    }
    else if (m_accountIndex.contains(_a->id()))
    {
        int idx = m_accountIndex[_a->id()];
        beginRemoveRows(QModelIndex(), idx, idx);
        m_accounts.removeAt(idx);
        m_accountIndex.remove(_a->id());

        for (int i = idx; i < m_accounts.size(); ++i)
        {
            m_accountIndex[m_accounts[i]->id()] = i;
        }

        endRemoveRows();
    }
}

