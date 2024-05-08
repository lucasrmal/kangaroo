#ifndef ACCOUNTPANE_H
#define ACCOUNTPANE_H

#include <QHash>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QWidget>

namespace KLib {
class ActionContainer;
class SimpleAccountController;
class Account;
enum class AccountClassification;
}  // namespace KLib

class AccountPaneTree : public QTreeView {
  Q_OBJECT

 public:
  AccountPaneTree(KLib::SimpleAccountController* _model,
                  QWidget* parent = nullptr);

  QHash<int, bool> expandedState() const;
  void restoreExpandedState(const QHash<int, bool>& _state);

 private slots:
  void onAccountTabChanged(KLib::Account* _account);

  void createAccount();
  void editAccount();
  void closeAccount();
  void reopenAccount();
  void removeAccount();

 protected:
  void drawBranches(QPainter* _painter, const QRect& _rect,
                    const QModelIndex& _index) const;
  void drawRow(QPainter* _painter, const QStyleOptionViewItem& _option,
               const QModelIndex& _index) const override;

  void mousePressEvent(QMouseEvent* _event) override;
  void mouseMoveEvent(QMouseEvent* _event) override;
  void keyPressEvent(QKeyEvent* _event) override;

 private:
  KLib::SimpleAccountController* m_model;

  QMenu* m_contextMenu_generic;
  QMenu* m_contextMenu_account;
  QAction* m_actCreate;
  QAction* m_actEdit;
  QAction* m_actClose;
  QAction* m_actRemove;

  KLib::Account* m_rightClickedAccount;

  static const QColor CATEGORY_COLOR;
  static const int MARGIN_LINE = 6;
};

class AccountPaneDelegate : public QStyledItemDelegate {
  Q_OBJECT

 public:
  AccountPaneDelegate(QObject* _parent = nullptr);

  QSize sizeHint(const QStyleOptionViewItem& _option,
                 const QModelIndex& _index) const override;

  void paint(QPainter* _painter, const QStyleOptionViewItem& _option,
             const QModelIndex& _index) const override;
};

class AccountPane : public QWidget {
  Q_OBJECT

 public:
  explicit AccountPane(QWidget* parent = nullptr);

  ~AccountPane();

 public slots:
  void createAccount();
  void editAccount();
  void closeAccount();
  void reopenAccount();
  void removeAccount();
  void reassignAllTransactions();
  void showClosedAccounts(bool _show);

 private slots:
  void onAccountTabChanged(KLib::Account* _account);
  void onItemSelected(const QModelIndex& _index);

 private:
  AccountPaneTree* m_treeAccounts;
  KLib::SimpleAccountController* m_model;
  KLib::Account* m_currentAccount;

  static KLib::ActionContainer* m_accountMenu;

  static QAction* m_actCreate;
  static QAction* m_actEdit;
  static QAction* m_actClose;
  static QAction* m_actReopen;
  static QAction* m_actRemove;
  static QAction* m_actReassign;
  static QAction* m_actShowClosed;
};

#endif  // ACCOUNTPANE_H
