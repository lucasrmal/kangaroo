#ifndef ACCOUNTTREEWIDGET_H
#define ACCOUNTTREEWIDGET_H

#include <QTreeView>
#include <QStyledItemDelegate>

namespace KLib
{
    class AccountSortFilterProxyModel;
    class Account;
    class AccountController;
}

class AccountTreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        AccountTreeDelegate(QObject* parent = nullptr)
            : QStyledItemDelegate(parent) {}

        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

        void paint(QPainter* _painter, const QStyleOptionViewItem& _option, const QModelIndex& _index) const override;
};

class AccountTreeWidget : public QTreeView
{
    Q_OBJECT

    public:
        explicit AccountTreeWidget(QWidget *parent = nullptr);

        AccountTreeWidget(KLib::Account* _topLevel,
                          bool _openAccountsOnly,
                          int _flags, QWidget *_parent = nullptr);

        KLib::Account* currentAccount() const;

        KLib::AccountSortFilterProxyModel* filterController() { return m_controller; }
        KLib::AccountController* controller() { return m_sourceModel; }

        void setContextMenu(QMenu* _menu);

    signals:
        void currentAccountChanged(KLib::Account* _account);

    protected:
        void selectionChanged(const QItemSelection& _selected, const QItemSelection& _deselected) override;

    public slots:
        void onSelectionChanged();
        void contextMenuRequested(QPoint pos);

    private:
        KLib::AccountSortFilterProxyModel* m_controller;
        KLib::AccountController* m_sourceModel;

        QMenu* m_contextMenu;

};

#endif // ACCOUNTTREEWIDGET_H
