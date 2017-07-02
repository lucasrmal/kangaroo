#ifndef LEDGERTAB_H
#define LEDGERTAB_H

#include <QWidget>
#include <QSet>
#include <QHash>
#include <QIcon>
#include <KangarooLib/amount.h>

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

        KLib::Account* account() const { return m_account; }

    signals:
        void changesSaved();
        void changesDiscarded();
        void editStarted();
        void editNewStarted();
        void selectedRowsChanged(const QList<int>& _rows);

    public slots:
        void onLedgerModified();

        void updateAccountInfos();
        void updateAccountBalance();
        void editAccount();        
        void selectAccountPicture();

        void updateBalance(const KLib::Amount& _amount);

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

        QAction* action_new() { return m_actNew; }
        QAction* action_edit() { return m_actEdit; }
        QAction* action_remove() { return m_actRemove; }
        QAction* action_enter() { return m_actEnter; }
        QAction* action_discard() { return m_actDiscard; }
        QAction* action_duplicate() { return m_actDuplicate; }
        QAction* action_splits() { return m_actSplits; }
        QAction* action_attachments() { return m_actAttachments; }
        QAction* action_find() { return m_actFind; }

    private slots:
        void newTransaction();
        void editTransaction();
        void deleteTransactions();
        void enterTransaction();
        void discardTransaction();
        void duplicateTransaction();
        void findTransactions();
        void editSplits();
        void editAttachments();

        void onCurrentTabChanged();

        void onChangesDiscarded();
        void onChangedSaved();
        void onEditStarted();
        void onEditNewStarted();
        void onSelectedRowsChanged(const QList<int>& _rows);

        void onAccountRemoved(KLib::Account* _a);
        void onAccountModified(KLib::Account* _a);

        void removeTab(QWidget* _tab);

    private:
        LedgerTabManager();

        void onLoad();
        void onUnload();
        void checkActions();

        bool askToContinue();

        KLib::ActionContainer* m_transacMenu;
        QAction* m_actNew;
        QAction* m_actEdit;
        QAction* m_actDuplicate;
        QAction* m_actRemove;
        QAction* m_actEnter;
        QAction* m_actDiscard;
        QAction* m_actSplits;
        QAction* m_actAttachments;
        QAction* m_actFind;

        QSet<LedgerTab*> m_tabs;
        QHash<int, LedgerTab*> m_tabIndex;
        LedgerTab* m_currentTab;

        QIcon m_icnEnter;
        QIcon m_icnEnterSchedule;

        static LedgerTabManager* m_instance;

        friend class HomeTabPlugin;
};

#endif // LEDGERTAB_H
