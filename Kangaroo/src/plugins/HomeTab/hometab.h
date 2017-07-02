#ifndef HOMETAB_H
#define HOMETAB_H

#include <QWidget>

class AccountTreeWidget;
class QVBoxLayout;

namespace KLib
{
    class ActionContainer;
    class Account;
}

class HomeTab : public QWidget
{
    Q_OBJECT
public:
    explicit HomeTab(QWidget *parent = 0);

    ~HomeTab();

signals:

public slots:
    void resetAccountTree();

    void createAccount();
    void editAccount();
    void closeAccount();
    void reopenAccount();
    void removeAccount();
    void openLedger();
    void showAllAccounts(bool _show);


private slots:
    void currentAccountChanged(KLib::Account* a);
    void onCurrentTabChanged();

private:
    void onLoad();
    void onUnload();

    AccountTreeWidget* m_accountTree;
    QVBoxLayout* m_layout;

    KLib::ActionContainer* m_accountMenu;

    QAction* m_actCreate;
    QAction* m_actOpen;
    QAction* m_actEdit;
    QAction* m_actClose;
    QAction* m_actReopen;
    QAction* m_actRemove;
    QAction* m_actShowAll;

    KLib::Account* m_currentAccount;

    friend class HomeTabPlugin;
};

#endif // HOMETAB_H
