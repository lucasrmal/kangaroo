/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  US
 */

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QLinkedList>
#include <QSet>
#include <QString>
#include <functional>
#include <vector>

#include "../amount.h"
#include "../interfaces/scriptable.h"
#include "properties.h"
#include "security.h"
#include "stored.h"

namespace KLib {

class Ledger;
class Account;

/**
 * @brief Classification of an account
 *
 * This is used by various widgets that display accounts that are not
 * Income/Expense/Trading.
 */
enum class AccountClassification {
  INVALID = -1,

  CASH_FLOW = 0,
  INVESTING,
  PROPERTY_DEBT,
  OTHER,

  NumCategories = 4
};

namespace AccountType {
enum Type {
  TOPLEVEL = 0,  // The parent of all accounts. This is a unique account.

  ASSET = 1,      // Generic asset
  LIABILITY = 2,  // Generic liability
  EQUITY = 3,     // Equity
  INCOME = 4,     // Income
  EXPENSE = 5,    // Expense
  TRADING = 6,    // For currency/security exchanges

  CASH = 10,         // Cash
  CHECKING = 11,     // Checking account
  INVESTMENT = 12,   // Investment (stock, bond, etf, etc.)
  DEPOSIT = 13,      // Security deposit
  PREPAIDCARD = 14,  // Prepaid cards, gift cards

  SAVINGS = 18,    // Savings account
  BROKERAGE = 19,  // Brokerage account

  CREDITCARD = 20,

  PROPERTY = 30,

  ACC_PAY = 40,
  ACC_REC = 41,
};
}

struct CustomType {
  int id;              // The identifier of the type. Must be >= START_ID_CUSTOM
  int generalType;     // The general type, ex: ASSET
  QSet<int> parents;   // The list of acceptable parents, ex: ASSET, CASH, BANK
  QSet<int> children;  // The list of acceptable children, ex: ASSET, CASH, BANK
  QString name;        // The name in the current app language
  QString icon;  // Name of the icon, same standard as Core::icon() accepts
  bool negativeDebits;  // If debits are negative, credits are positive (ie:
                        // liability), then true, otherwise false (ie: asset)
  AccountClassification classification;  // The account category for this type

  int weight;  // The weight of the account type when displayed in a sorted list
               // with accounts of other types. Lighter (smaller) values rise,
               // heavier (greater) values sink.

  QString customDebit;   // How is "DEBIT" referred to
  QString customCredit;  // How is "CREDIT" referred to
  std::function<bool(const Account*, QString&)>
      newValidation;  // Validation function for new accounts.
  std::function<bool(const Account*, QString&)>
      deleteValidation;  // Validation function before deleting an account.

  static const int
      START_ID_CUSTOM;  // Smallest account type ID possible for custom types
};

namespace AccountTypeFlags {
enum Flags {
  Flag_Toplevel = 0x01,
  Flag_Asset = 0x02,  // Excludes investments
  Flag_Liability = 0x04,
  Flag_Equity = 0x08,
  Flag_Income = 0x10,
  Flag_Expense = 0x20,
  Flag_Trading = 0x40,
  Flag_Investment = 0x80,

  Flag_All = 0xFF,
  Flag_None = 0x00,
  Flag_IncomeExpense = Flag_Income | Flag_Expense,
  Flag_AllAssets = Flag_Asset | Flag_Investment,
  Flag_AllButInvTrad = Flag_All & ~(Flag_Investment | Flag_Trading)
};
}

class Account : public IStored {
  Q_OBJECT
  K_SCRIPTABLE(Account)

  Q_PROPERTY(bool hasParent READ hasParent)
  Q_PROPERTY(int childCount READ childCount)

  Q_PROPERTY(int type READ type WRITE setType)
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(QString mainCurrency READ mainCurrency WRITE setCurrency)
  Q_PROPERTY(QString code READ code WRITE setCode)
  Q_PROPERTY(QString note READ note WRITE setNote)
  Q_PROPERTY(bool isPlaceholder READ isPlaceholder WRITE setIsPlaceholder)
  Q_PROPERTY(bool isOpen READ isOpen WRITE setOpen)
  Q_PROPERTY(int idInstitution READ idInstitution WRITE setIdInstitution)
  Q_PROPERTY(int idSecurity READ idSecurity WRITE setIdSecurity)
  Q_PROPERTY(int idPicture READ idPicture WRITE setIdPicture)

 public:
  class Key {
    bool priv;
    Key(int _priv) : priv(_priv) {}

    friend class Account;
  };

  static Key publicKey;

  virtual ~Account();

  Q_INVOKABLE KLib::Account* parent() const;
  Q_INVOKABLE void moveToParent(KLib::Account* _parent);

  /**
    Moves to _parent and changes the type to _newType.

    This only works for converting Asset to/from Liability, Income to/from
    Expense
  */
  Q_INVOKABLE void moveToParent(KLib::Account* _parent, int _newType);

  int childCount() const { return m_children.size(); }
  const std::vector<KLib::Account*>& getChildren() const { return m_children; }

  /**
   * @brief getAccount returns an account by ID if it exists, otherwise throws
   * an exception. This is O(1)
   */
  static KLib::Account* getAccount(int id);

  Q_INVOKABLE KLib::Account* getChild(int _id) const;
  Q_INVOKABLE KLib::Account* getChild(const QString& _code) const;
  Q_INVOKABLE QString getPath(int _id, int _upToHeight = 0) const;

  Q_INVOKABLE KLib::Account* addChild(const QString& _name, int _type,
                                      const QString& _currency, int _idSecurity,
                                      bool _placeholder = false,
                                      int _idInstitution = Constants::NO_ID,
                                      const QString& _code = QString(),
                                      const QString& _note = QString(),
                                      const Key& _k = publicKey);

  /**
    @brief Removes the account _id, which muse be a direct child of this
    account.
  */
  Q_INVOKABLE bool removeChild(int _id, const Key& _k = publicKey);

  Q_INVOKABLE bool canBeRemoved() const;
  Q_INVOKABLE bool canBeClosed() const;

  bool isAncestorOf(const Account* _account) const;
  bool isDescendantOf(const Account* _account) const;

  bool hasParent() const;
  int type() const { return m_type; }
  QString name() const {
    if (m_idSecurity != Constants::NO_ID) {
      return SecurityManager::instance()->get(m_idSecurity)->formattedName();
    }

    return m_name;
  }
  QString mainCurrency() const { return m_mainCurrency; }
  QString code() const { return m_code; }
  QString note() const { return m_note; }
  bool isPlaceholder() const { return m_isPlaceholder; }
  bool isOpen() const { return m_isOpen; }
  int idInstitution() const { return m_idInstitution; }
  int idSecurity() const { return m_idSecurity; }
  int idPicture() const { return m_idPicture; }
  int idDefaultDividendAccount() const { return m_idDefaultDividendAccount; }

  const QSet<QString>& secondaryCurrencies() const {
    return m_secondaryCurrencies;
  }
  std::vector<QString> allCurrencies() const;
  bool supportsCurrency(const QString& _currency) const;

  void setType(int _type);
  void setName(const QString& _name);
  void setCurrency(const QString& _currency);
  void setSecondaryCurrencies(const QSet<QString>& _currencies);
  void setCode(const QString& _code);
  void setNote(const QString& _note);
  void setOpen(bool _open);
  void setIsPlaceholder(bool _isPlaceholder);
  void setIdInstitution(int _id);
  void setIdSecurity(int _id);
  void setIdPicture(int _id);
  void setIdDefaultDividendAccount(int id);

  QPixmap defaultPicture(bool _thumbnail = false) const;

  Q_INVOKABLE KLib::Amount balanceToday() const;
  Q_INVOKABLE KLib::Amount treeValueToday() const;

  Q_INVOKABLE KLib::Amount balance() const;
  Q_INVOKABLE KLib::Amount treeValue() const;

  /**
    Balance at end of _date
  */
  Q_INVOKABLE KLib::Amount balanceAt(const QDate& _date) const;

  /**
    Tree value (in file currency) at end of _date
  */
  Q_INVOKABLE KLib::Amount treeValueAt(const QDate& _date) const;

  Q_INVOKABLE KLib::Amount balanceBetween(const QDate& _start,
                                          const QDate& _end) const;

  /**
    Value of account and all subaccounts, in file currency (currency of toplevel
    account)
  */
  Q_INVOKABLE KLib::Amount treeValueBetween(const QDate& _start,
                                            const QDate& _end) const;

  Q_INVOKABLE QString formatAmount(const KLib::Amount& _amount) const;

  Q_INVOKABLE KLib::Ledger* ledger() const {
    return m_isPlaceholder ? nullptr : m_ledger;
  }
  Q_INVOKABLE KLib::Properties* properties() const { return m_properties; }

  Q_INVOKABLE void doneHoldToModify() override;

  Q_INVOKABLE KLib::Account* account(int _id) const {
    return m_accounts.value(_id);
  }
  Q_INVOKABLE const QHash<int, KLib::Account*>& accounts() const {
    return m_accounts;
  }

  static Account* getTopLevel();
  static Account* createCurrencyTradingAccount(const QString& _currency);
  static Account* createSecurityTradingAccount(int _idSecurity);
  static Account* getFromPath(const QString& _path);

  static bool accountIsCurrencyTrading(int _id);

  static bool possibleType(int _childType, int _parentType);
  static QList<int> possibleTypes(int _parentType);
  static int generalType(int _type);
  static QString typeToString(int _type);
  static QString typeToIcon(int _type);
  static void debitCreditLabelsFor(int _type, QString& _debit,
                                   QString& _credit);
  static bool negativeDebits(int _type);
  static int accountTypeWeight(int _type);
  static AccountClassification classificationForType(int _type);

  static bool matchesFlags(const Account* _a, int _flags);

  static bool addCustomType(CustomType* _custom, QString& _errorMsg);

  static const char PATH_SEPARATOR;

 signals:
  void accountAdded(KLib::Account* a);
  void accountRemoved(KLib::Account* a);
  void accountModified(KLib::Account* a);

 protected:
  void load(QXmlStreamReader& _reader) override;
  void afterLoad() override;
  void save(QXmlStreamWriter& _writer) const override;
  void unload() override;

 private slots:
  void onPropertiesModified();

 private:
  Account();

  static void setupTopLevel();
  static void setupAccount(Account* a);
  static void checkRootTradingAccount();

  void rec_load(QXmlStreamReader& _reader, Account* _parent);
  void rec_save(QXmlStreamWriter& _writer) const;

  int m_type;

  QString m_name;
  QString m_mainCurrency;
  QSet<QString> m_secondaryCurrencies;
  QString m_code;
  QString m_note;
  bool m_isPlaceholder;
  bool m_isOpen;

  int m_idInstitution;
  int m_idSecurity;
  int m_idPicture;
  int m_idDefaultDividendAccount;

  std::vector<Account*> m_children;
  Account* m_parent;

  Ledger* m_ledger;
  Properties* m_properties;
  Account* m_topTradingAccount;

  static Account* m_topLevel;
  static int m_nextId;
  static QHash<int, Account*> m_accounts;
  static QHash<int, CustomType*> m_customTypes;
};

}  // namespace KLib

Q_DECLARE_METATYPE(KLib::Account*)

#endif  // ACCOUNT_H
