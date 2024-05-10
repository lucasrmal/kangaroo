#include "accountbalancesovertime.h"

#include <KangarooLib/model/account.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/ledger.h>
#include <KangarooLib/model/pricemanager.h>
#include <KangarooLib/model/security.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/widgets/accountselector.h>
#include <KangarooLib/ui/widgets/returnchart.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QVBoxLayout>

using namespace KLib;

AccountBalancesOverTime::AccountBalancesOverTime(QWidget* _parent)
    : IChart(_parent), m_model(new QStandardItemModel(this)) {
  m_chart = new ReturnChart(ReturnChartStyle::Line, ValueType::Amount,
                            Qt::EditRole, this);
  m_chart->setModel(m_model);
  m_chart->setLegendVisible(false);

  m_selector = new AccountSelector(
      {.selectorFlags = AccountSelectorFlags::Flag_IncludePlaceholders,
       .typeFlags = AccountTypeFlags::Flag_AllAssets |
                    AccountTypeFlags::Flag_Liability |
                    AccountTypeFlags::Flag_Equity},
      this);

  m_chkIncludeSubtree = new QCheckBox(tr("Include Sub&tree"), this);
  m_chkShowAverage = new QCheckBox(tr("Show A&verage"), this);

  m_cboDisplayType = new QComboBox(this);
  m_cboMonth = new QComboBox(this);
  m_spinYear = new QSpinBox(this);

  m_btnRefresh =
      new QPushButton(Core::icon("view-refresh"), tr("Refresh"), this);

  m_spinYear->setMinimum(1900);
  m_spinYear->setMaximum(9999);

  for (int i = 1; i <= 12; ++i) m_cboMonth->addItem(QDate::longMonthName(i));

  m_cboMonth->setCurrentIndex(QDate::currentDate().month() - 1);
  m_spinYear->setValue(QDate::currentDate().year());

  m_cboDisplayType->addItem(tr("YTD"), (int)PeriodType::YTD);
  m_cboDisplayType->addItem(tr("YTD 2 Year Comp"),
                            (int)PeriodType::YTDTwoYearsComp);
  m_cboDisplayType->addItem(tr("YTD 3 Year Comp"),
                            (int)PeriodType::YTDThreeYearsComp);
  m_cboDisplayType->addItem(tr("Year"), (int)PeriodType::Year);
  m_cboDisplayType->addItem(tr("2 Year Comp"), (int)PeriodType::TwoYearsComp);
  m_cboDisplayType->addItem(tr("3 Year Comp"), (int)PeriodType::ThreeYearsComp);
  m_cboDisplayType->addItem(tr("Month"), (int)PeriodType::Month);
  m_cboDisplayType->addItem(tr("3 Years"), (int)PeriodType::ThreeYears);
  m_cboDisplayType->addItem(tr("5 Years"), (int)PeriodType::FiveYears);
  m_cboDisplayType->addItem(tr("10 Years"), (int)PeriodType::TenYears);

  m_cboDisplayType->setMaximumWidth(200);

  QLabel* lblAccount = new QLabel(tr("&Account:"), this);
  QLabel* lblPeriod = new QLabel(tr("&Period:"), this);

  lblAccount->setBuddy(m_selector);
  lblPeriod->setBuddy(m_cboDisplayType);

  QHBoxLayout* controlsLayout = new QHBoxLayout();
  controlsLayout->addWidget(lblAccount);
  controlsLayout->addWidget(m_selector);
  controlsLayout->addWidget(m_chkIncludeSubtree);
  controlsLayout->addWidget(m_chkShowAverage);
  controlsLayout->addSpacing(10);
  controlsLayout->addWidget(lblPeriod);
  controlsLayout->addWidget(m_cboDisplayType);
  controlsLayout->addWidget(m_cboMonth);
  controlsLayout->addWidget(m_spinYear);
  controlsLayout->addSpacing(10);
  controlsLayout->addWidget(m_btnRefresh);
  controlsLayout->addStretch(2);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(controlsLayout);
  mainLayout->addWidget(m_chart);

  connect(m_btnRefresh, &QPushButton::clicked, this,
          &AccountBalancesOverTime::refresh);
  connect(m_cboDisplayType, SIGNAL(currentIndexChanged(int)), this,
          SLOT(updatePeriodSelection()));
  connect(m_selector, SIGNAL(currentAccountChanged(KLib::Account*)), this,
          SLOT(updatePeriodSelection()));

  updatePeriodSelection();
}

void AccountBalancesOverTime::updatePeriodSelection() {
  PeriodType period = (PeriodType)m_cboDisplayType->currentData().toInt();

  m_cboMonth->setEnabled(period == PeriodType::Month);
  m_spinYear->setEnabled(period != PeriodType::YTD &&
                         period != PeriodType::YTDTwoYearsComp &&
                         period != PeriodType::YTDThreeYearsComp);

  m_btnRefresh->setEnabled(m_selector->currentAccount());
}

void AccountBalancesOverTime::refresh() {
  if (!m_selector->currentAccount()) return;

  m_chart->setAverageVisible(m_chkShowAverage->isChecked());

  PeriodType type = PeriodType(m_cboDisplayType->currentData().toInt());

  int interval[3] = {0, 0, 0};  // Days, Months, Years
  int numDataPoints = 0;
  QDate beginDate;
  QString title;
  int numCols = 1;

  switch (type) {
    case PeriodType::Month:
      interval[0] = 1;
      beginDate = QDate(m_spinYear->value(), m_cboMonth->currentIndex() + 1, 1);
      numDataPoints = beginDate.daysInMonth();
      title = tr("%3 - %1 %2")
                  .arg(QDate::longMonthName(beginDate.month()))
                  .arg(beginDate.year());
      break;

    case PeriodType::YTD:
      interval[1] = 1;
      numDataPoints = QDate::currentDate().month();
      beginDate = QDate(QDate::currentDate().year(), 1, 1);
      title = tr("%2 - YTD %1").arg(beginDate.year());
      break;

    case PeriodType::YTDTwoYearsComp:
      interval[1] = 1;
      numDataPoints = QDate::currentDate().month();
      beginDate = QDate(QDate::currentDate().year() - 1, 1, 1);
      title = tr("%3 - YTD %1-%2")
                  .arg(beginDate.year())
                  .arg(QDate::currentDate().year());
      numCols = 2;
      break;

    case PeriodType::YTDThreeYearsComp:
      interval[1] = 1;
      numDataPoints = QDate::currentDate().month();
      beginDate = QDate(QDate::currentDate().year() - 2, 1, 1);
      title = tr("%3 - YTD %1-%2")
                  .arg(beginDate.year())
                  .arg(QDate::currentDate().year());
      numCols = 3;
      break;

    case PeriodType::Year:
      interval[1] = 1;
      numDataPoints = 12;
      beginDate = QDate(m_spinYear->value(), 1, 1);
      title = tr("%2 - %1").arg(beginDate.year());
      break;

    case PeriodType::TwoYearsComp:
      interval[1] = 1;
      numDataPoints = 12;
      beginDate = QDate(m_spinYear->value() - 1, 1, 1);
      title = tr("%3 - %1-%2").arg(beginDate.year()).arg(m_spinYear->value());
      numCols = 2;
      break;

    case PeriodType::ThreeYearsComp:
      interval[1] = 1;
      numDataPoints = 12;
      beginDate = QDate(m_spinYear->value() - 2, 1, 1);
      title = tr("%3 - %1-%2").arg(beginDate.year()).arg(m_spinYear->value());
      numCols = 3;
      break;

    case PeriodType::ThreeYears:
      interval[2] = 1;
      numDataPoints = 3;
      beginDate = QDate(m_spinYear->value() - 2, 1, 1);
      title = tr("%3 - %1-%2").arg(beginDate.year()).arg(m_spinYear->value());
      break;

    case PeriodType::FiveYears:
      interval[2] = 1;
      numDataPoints = 5;
      beginDate = QDate(m_spinYear->value() - 4, 1, 1);
      title = tr("%3 - %1-%2").arg(beginDate.year()).arg(m_spinYear->value());
      break;

    case PeriodType::TenYears:
      interval[2] = 1;
      numDataPoints = 10;
      beginDate = QDate(m_spinYear->value() - 9, 1, 1);
      title = tr("%3 - %1-%2").arg(beginDate.year()).arg(m_spinYear->value());
      break;
  }

  m_model->setColumnCount(
      numCols);  // Do not want to display stuff while we update
  m_model->setRowCount(numDataPoints);

  Account* account = m_selector->currentAccount();
  QString currency;

  try {
    currency = m_chkIncludeSubtree->isChecked()
                   ? Account::getTopLevel()->mainCurrency()
                   : account->mainCurrency();

    if (currency.isEmpty())  // Security
    {
      currency =
          SecurityManager::instance()->get(account->idSecurity())->currency();
    }

    m_chart->setYAxisIndicator(
        CurrencyManager::instance()->get(currency)->symbol(), true);
  } catch (...) {
  }

  m_chart->setTitle(title.arg(account->name()) +
                    QString(" (%1)").arg(currency));
  m_chart->setLegendVisible(numCols > 1);

  if (type == PeriodType::TwoYearsComp || type == PeriodType::ThreeYearsComp ||
      type == PeriodType::YTDTwoYearsComp ||
      type == PeriodType::YTDThreeYearsComp) {
    for (int col = 0; col < numCols; ++col) {
      m_model->setHeaderData(col, Qt::Horizontal, beginDate.year() + col);
    }
  }

  auto getAmount = [account, currency, this](const QDate& _date) {
    try {
      if (m_chkIncludeSubtree->isChecked()) {
        return account->treeValueAt(_date).toDouble();
      } else if (account->idSecurity() != Constants::NO_ID) {
        return PriceManager::instance()->rate(account->idSecurity(), currency,
                                              _date) *
               account->balanceAt(_date).toDouble();
      } else {
        return account->balanceAt(_date).toDouble();
      }
    } catch (...) {
      return 0.0;
    }
  };

  for (int i = 0; i < numDataPoints; ++i) {
    QString caption;

    switch (type) {
      case PeriodType::Month:
        caption = QString("%1/%2").arg(beginDate.month()).arg(beginDate.day());
        break;

      case PeriodType::YTD:
      case PeriodType::YTDTwoYearsComp:
      case PeriodType::YTDThreeYearsComp:
      case PeriodType::Year:
      case PeriodType::TwoYearsComp:
      case PeriodType::ThreeYearsComp:
        caption = QDate::longMonthName(beginDate.month());
        break;

      case PeriodType::ThreeYears:
      case PeriodType::FiveYears:
      case PeriodType::TenYears:
        caption = QString::number(beginDate.year());
        break;
    }

    QDate endDate = beginDate.addDays(interval[0])
                        .addMonths(interval[1])
                        .addYears(interval[2]);

    switch (type) {
      case PeriodType::YTDTwoYearsComp:
      case PeriodType::YTDThreeYearsComp:
      case PeriodType::TwoYearsComp:
      case PeriodType::ThreeYearsComp:
        for (int j = 0; j < numCols; ++j) {
          m_model->setData(m_model->index(i, j),
                           getAmount(endDate.addYears(j).addDays(-1)),
                           Qt::EditRole);
        }
        break;

      default:
        m_model->setData(m_model->index(i, 0), getAmount(endDate.addDays(-1)),
                         Qt::EditRole);
    }

    m_model->setHeaderData(i, Qt::Vertical, caption);

    beginDate = endDate;
  }

  // m_model->setColumnCount(1); //Now display!
}
