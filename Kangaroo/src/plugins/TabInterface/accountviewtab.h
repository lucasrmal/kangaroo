#ifndef ACCOUNTVIEWTAB_H
#define ACCOUNTVIEWTAB_H

#include <QWidget>

class AccountTreeWidget;
class QVBoxLayout;
class QMenu;
class QToolBar;

namespace KLib
{
    class Account;
}

class AccountViewTab : public QWidget
{
    Q_OBJECT

    protected:
        explicit AccountViewTab(KLib::Account* _topLevel, bool _showOpenOnly, int _flags, QWidget *parent = nullptr);

    public:

        enum class ActionType
        {
            Create,
            Open,
            Edit,
            CloseReopen,
            Delete,
            ShowAll,
            Find
        };

        virtual ~AccountViewTab() {}

        QAction* action(ActionType _type) const;

    public slots:
        void resetAccountTree();

        void createAccount();
        void editAccount();
        void closeOpenAccount();
        void deleteAccount();
        void openLedger();
        void showAllAccounts(bool _show);
        void findAccounts();
        void doneFindAccounts();

        void filterAccounts(const QString& _filter);
        void setIncludeChildrenInFilter(bool include);

    private slots:
        void onCurrentAccountChanged(KLib::Account* a);

    protected:
        void setToolBar(QToolBar* _toolbar);

        QToolBar* toolBar() const { return m_toolbar; }

        AccountTreeWidget* m_accountTree;

    private:
        QMenu* m_accountMenu;
        QToolBar* m_toolbar;
        QVBoxLayout* m_layout;

        QAction* m_actCreate;
        QAction* m_actOpen;
        QAction* m_actEdit;
        QAction* m_actCloseOpen;
        QAction* m_actDelete;
        QAction* m_actShowAll;
        QAction* m_actFind;

        QWidget* m_findBar;

        KLib::Account* m_currentAccount;

};

#endif // ACCOUNTVIEWTAB_H
