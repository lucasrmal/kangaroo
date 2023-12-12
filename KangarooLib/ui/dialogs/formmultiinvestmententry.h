#ifndef FORMMULTIINVESTMENTENTRY_H
#define FORMMULTIINVESTMENTENTRY_H

#include <QAbstractTableModel>
#include <QLinkedList>
#include <QStyledItemDelegate>

#include "../../amount.h"
#include "camsegdialog.h"

class QDateEdit;
class QLabel;
class QTableView;

namespace KLib {

class Account;
class Currency;
class Security;

struct RowInfo {
  RowInfo(Account* _account);

  QString memo;
  Amount shares;
  Amount amount;
  Amount pricePerShare;

  const Account* account() const { return m_account; }
  const Security* security() const { return m_security; }
  const Currency* currency() const { return m_currency; }

 private:
  Account* m_account;
  Security* m_security;
  Currency* m_currency;
};

class MultiInvestmentModel : public QAbstractTableModel {
  Q_OBJECT

 public:
  enum Columns {
    COLUMN_ACCOUNT,
    COLUMN_MEMO,
    COLUMN_SHARES,
    COLUMN_PRICE_PER_SHARE,
    COLUMN_AMOUNT,

    NumColumns
  };

  MultiInvestmentModel(const QLinkedList<Account*>& _accounts,
                       QObject* _parent);
  ~MultiInvestmentModel() override {}

  int rowCount(const QModelIndex& _parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& _parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& _index, int _role) const override;

  QVariant headerData(int _section, Qt::Orientation _orientation,
                      int _role = Qt::DisplayRole) const override;

  Qt::ItemFlags flags(const QModelIndex& _index) const override;

  bool setData(const QModelIndex& _index, const QVariant& _value,
               int _role = Qt::EditRole) override;

  void save(const QDate& _date, const Account* m_brokerageAccount);

  const QList<RowInfo>& rowInfo() const { return m_rows; }

 private:
  QList<RowInfo> m_rows;
};

class FormMultiInvestmentEntry : public CAMSEGDialog {
  Q_OBJECT

 public:
  FormMultiInvestmentEntry(int _brokerageAccountId, QWidget* _parent);
  ~FormMultiInvestmentEntry() override {}

  void accept() override;

  QSize sizeHint() const override { return QSize(850, 600); }

 private slots:
  void updateTotal();

 private:
  void createModel();

  Account* m_brokerageAccount;

  QDateEdit* m_dteEdit;
  QLabel* m_lblTotal;
  QTableView* m_transactionTable;
  MultiInvestmentModel* m_model;
};

class MultiInvestmentEntryDelegate : public QStyledItemDelegate {
  Q_OBJECT

 public:
  MultiInvestmentEntryDelegate(const MultiInvestmentModel* _model,
                               QObject* _parent);

  ~MultiInvestmentEntryDelegate() override {}

  /* OVERLOADED VIRTUAL METHODS */

  QWidget* createEditor(QWidget* _parent, const QStyleOptionViewItem& _option,
                        const QModelIndex& _index) const override;

  void setEditorData(QWidget* _editor,
                     const QModelIndex& _index) const override;

  void setModelData(QWidget* _editor, QAbstractItemModel* _model,
                    const QModelIndex& _index) const override;

  QSize sizeHint(const QStyleOptionViewItem& _option,
                 const QModelIndex& _index) const override;

  void updateEditorGeometry(QWidget* _editor,
                            const QStyleOptionViewItem& _option,
                            const QModelIndex& _index) const override;

 private:
  const MultiInvestmentModel* m_model;
  int m_rowHeight;
};

}  // namespace KLib

#endif  // FORMMULTIINVESTMENTENTRY_H
