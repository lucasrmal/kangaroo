#ifndef ACCOUNTACTIONS_H
#define ACCOUNTACTIONS_H

namespace KLib
{
    class Account;
}

class AccountActions
{
    public:
        static void createAccount(KLib::Account* _parent);
        static void editAccount(KLib::Account* _account);
        static bool closeAccount(KLib::Account* _account);
        static bool reopenAccount(KLib::Account* _account);
        static bool removeAccount(KLib::Account* _account);
};

#endif // ACCOUNTACTIONS_H
