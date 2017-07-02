#ifndef ACCOUNTEDITTAB_H
#define ACCOUNTEDITTAB_H

#include <QWidget>

namespace KLib
{
    class Account;
}

class AccountEditTab : public QWidget
{
    Q_OBJECT

    public:
        explicit AccountEditTab(QWidget* _parent) : QWidget(_parent) {}

        virtual QString tabName() const = 0;
        virtual QString tabIcon() const = 0;

        /**
         * @brief enableIf
         * @param _accountType The currently selected account type
         * @return Returns true if the tab should be enabled for this account type.
         */
        virtual bool enableIf(int _accountType) const = 0;

        /**
         * @brief Loads data from _account
         * @param _account
         */
        virtual void fillData(const KLib::Account* _account) = 0;

        /**
         * @brief Saves the changes to _account. No need to catch ModelException, it will be caught by FormAccountEdit.
         * @param _account
         */
        virtual void save(KLib::Account* _account) const = 0;

        /**
         * @brief validate Field validation
         * @return Empty list if the validation is succesfull otherwise, list containing the error messages
         */
        virtual QStringList validate() const = 0;

        virtual void onAccountTypeChanged() {}
        virtual void onParentAccountChanged() {}

};

///**
// * @brief The AccountEditTabFactorySuper class
// *
// * Do not use this class, it's for internal use of
// * FormEditAccount. Use AccountEditTabFactory instead.
// *
// * @see AccountEditTabFactory
// */
//class AccountEditTabFactorySuper
//{
//    public:
//        virtual AccountEditTab* buildTab(QWidget* _parent) const = 0;
//};

///**
// * Use this class to add register a AccountEditTab to FormEditAccount.
// *
// * Simply pass <code>new AccountEditTabFactory<MyOwnAccountEditTab>()</code> to <code>FormEditAccount::registerTab()</code>.
// */
//template<class T>
//class AccountEditTabFactory : public AccountEditTabFactorySuper
//{
//    public:
//        AccountEditTab* buildTab(QWidget* _parent) const
//        {
//            return new T(_parent);
//        }
//};

#endif // ACCOUNTEDITTAB_H
