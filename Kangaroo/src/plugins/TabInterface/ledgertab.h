#ifndef LEDGERTAB_H
#define LEDGERTAB_H

#include <QWidget>
#include <QSet>
#include <QHash>
#include <QIcon>
#include <KangarooLib/amount.h>
#include <KangarooLib/model/ledger.h>

namespace KLib
{
    class Account;
    class LedgerWidget;
    class ActionContainer;
}

class QLabel;
class QPushButton;

class LedgerTab : public QWidget
{
    Q_OBJECT

    public:
        LedgerTab(KLib::Account* _account, QWidget *parent = 0);

        LedgerTab() {}

        KLib::Account* account() const { return m_account; }

    signals:
        void changesSaved();
        void changesDiscarded();
        void editStarted();
        void editNewStarted();
        void showMessage(const QString& _message);

    public slots:
        void onLedgerModified();

        void updateAccountInfos();
        void updateAccountBalance();
        void editAccount();        
        void selectAccountPicture();

        void updateBalance(const KLib::Amount& _amount);

    private:
        void onMessageReceived(const QString& _message);
        void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

    private:
        KLib::Account* m_account;
        KLib::LedgerWidget* m_ledgerWidget;

        QPushButton* m_lblPic;
        QLabel* m_lblAccountName;
        QLabel* m_lblAccountType;
        QLabel* m_lblAccountBalance;
        QLabel* m_lblAccountValue;
        //QPushButton* m_btnEditAccount;

        static bool m_actionsCreated;

        friend class LedgerTabManager;
};

/**
 * @brief The LedgerWidgetManager class
 *
 * Redirects the actions received to the active LedgerWidget if it exists.
 */
class LedgerTabManager : public QObject
{
    Q_OBJECT

    public:
        static LedgerTabManager* instance();

        void openLedger(KLib::Account* _a);
        void closeLedger(KLib::Account* _a);

        void registerLedger(LedgerTab* _tab);

        QAction* action_find() { return m_actFind; }

    public slots:
        void setStatusBarMessage(const QString& _message);

    signals:
        void currentAccountChanged(KLib::Account* _account) const;

    private slots:
        void findTransactions();

        void onCurrentTabChanged();

        void onAccountRemoved(KLib::Account* _a);
        void onAccountModified(KLib::Account* _a);

        void onBalanceTodayChanged(int _idAccount, const KLib::Balances& _balances);

        void removeTab(QWidget* _tab);

    private:
        LedgerTabManager();

        void onLoad();
        void onUnload();

        bool askToContinue();

        KLib::ActionContainer* m_transacMenu;
        QAction* m_actFind;

        QLabel* m_lblStatusMessage;

        QSet<LedgerTab*> m_tabs;
        QHash<int, LedgerTab*> m_tabIndex; //Account ID, tab
        LedgerTab* m_currentTab;

        QIcon m_icnEnter;
        QIcon m_icnEnterSchedule;

        static LedgerTabManager* m_instance;

        friend class TabInterfacePlugin;
};

#endif // LEDGERTAB_H
