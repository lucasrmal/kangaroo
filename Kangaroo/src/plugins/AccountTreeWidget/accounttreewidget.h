#ifndef ACCOUNTTREEWIDGET_H
#define ACCOUNTTREEWIDGET_H

#include <QTreeView>
#include <QItemDelegate>

namespace KLib
{
    class AccountSortFilterProxyModel;
    class Account;
}

class AccountTreeDelegate : public QItemDelegate
{
    Q_OBJECT

    public:
        AccountTreeDelegate(QObject* parent = 0) : QItemDelegate(parent) {}

        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const;
};

class AccountTreeWidget : public QTreeView
{
    Q_OBJECT

    public:
        explicit AccountTreeWidget(QWidget *parent = 0);

        void resetModel(bool _openOnly = true);

        KLib::Account* currentAccount() const;

    signals:
        void currentAccountChanged(KLib::Account* _account);

    protected:
        void selectionChanged(const QItemSelection& _selected, const QItemSelection& _deselected);

    public slots:
        void onSelectionChanged();
        void contextMenuRequested(QPoint pos);

    private:
        KLib::AccountSortFilterProxyModel* m_controller;

};

#endif // ACCOUNTTREEWIDGET_H
