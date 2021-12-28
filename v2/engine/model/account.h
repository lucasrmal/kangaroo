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

#include <functional>
#include <string>
#include <vector>

#include "model/account.h"
#include "model/decimal-number.h"
#include "model/ledger.h"
#include "model/properties.h"
#include "model/stored.h"

namespace kangaroo {

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
  std::vector<int> parents;   // The list of acceptable parents, ex: ASSET, CASH, BANK
  std::vector<int> children;  // The list of acceptable children, ex: ASSET, CASH, BANK
  std::string name;        // The name in the current app language
  std::string icon;  // Name of the icon, same standard as Core::icon() accepts
  bool negativeDebits;  // If debits are negative, credits are positive (ie:
                        // liability), then true, otherwise false (ie: asset)
  AccountClassification classification;  // The account category for this type

  int weight;  // The weight of the account type when displayed in a sorted list
               // with accounts of other types. Lighter (smaller) values rise,
               // heavier (greater) values sink.

  std::string customDebit;   // How is "DEBIT" referred to
  std::string customCredit;  // How is "CREDIT" referred to
  std::function<bool(const Account*, std::string&)>
      newValidation;  // Validation function for new accounts.
  std::function<bool(const Account*, std::string&)>
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
 public:
  class Key {
    bool priv;
    Key(int _priv) : priv(_priv) {}

    friend class Account;
  };

  static Key publicKey;

  virtual ~Account();

  Account* parent() const;
  void moveToParent(Account* _parent);

  /**
    Moves to _parent and changes the type to _newType.

    This only works for converting Asset to/from Liability, Income to/from
    Expense
  */
  void moveToParent(Account* _parent, int _newType);

  int childCount() const { return children_.count(); }
  const std::vector<Account*> getChildren() const {
    return children_;
  }

  Account* getChild(int _id) const;
  Account* getChild(const std::string& _code) const;
  std::string getPath(int _id, int _upToHeight = 0) const;

  Account* addChild(const std::string& _name, int _type,
                                      const std::string& _currency, int _idSecurity,
                                      bool _placeholder = false,
                                      int _idInstitution = Constants::NO_ID,
                                      const std::string& _code = "",
                                      const std::string& _note = "",
                                      const Key& _k = publicKey);

  /**
    @brief Removes the account _id, which must be a direct child of this
    account.
  */
  bool removeChild(int _id, const Key& _k = publicKey);

  bool canBeRemoved() const;
  bool canBeClosed() const;

  bool isAncestorOf(const Account* _account) const;
  bool isDescendantOf(const Account* _account) const;

  bool hasParent() const;
  int type() const { return type_; }
  std::string name() const { return name_; }
  std::string mainCurrency() const { return mainCurrency_; }
  std::string code() const { return code_; }
  std::string note() const { return note_; }
  bool isPlaceholder() const { return isPlaceholder_; }
  bool isOpen() const { return isOpen_; }
  int idInstitution() const { return idInstitution_; }
  int idSecurity() const { return idSecurity_; }
  int idPicture() const { return idPicture_; }

  const std::vector<std::string>& secondaryCurrencies() const {
    return secondaryCurrencies_;
  }
  std::vector<std::string> allCurrencies() const;

  void setType(int _type);
  void setName(const std::string& _name);
  void setCurrency(const std::string& _currency);
  void setSecondaryCurrencies(const std::vector<std::string>& _currencies);
  void setCode(const std::string& _code);
  void setNote(const std::string& _note);
  void setOpen(bool _open);
  void setIsPlaceholder(bool _isPlaceholder);
  void setIdInstitution(int _id);
  void setIdSecurity(int _id);
  void setIdPicture(int _id);

  Icon defaultPicture(bool _thumbnail = false) const;

  DecimalNumber balanceToday() const;
  DecimalNumber treeValueToday() const;

  DecimalNumber balance() const;
  DecimalNumber treeValue() const;

  /**
    Balance at end of _date
  */
  DecimalNumber balanceAt(const Date& _date) const;

  /**
    Tree value (in file currency) at end of _date
  */
  DecimalNumber treeValueAt(const Date& _date) const;

  DecimalNumber balanceBetween(const Date& _start,
                                          const Date& _end) const;

  /**
    Value of account and all subaccounts, in file currency (currency of toplevel
    account)
  */
  DecimalNumber treeValueBetween(const Date& _start,
                                            const Date& _end) const;

  std::string formatAmount(const DecimalNumber& _amount) const;

  Ledger* ledger() const {
    return isPlaceholder_ ? nullptr : ledger_;
  }
  Properties* properties() const { return properties_; }

  void doneHoldToModify();

  Account* account(int _id) const {
    return accounts_.value(_id);
  }
  const std::unordered_map<int, Account*>& accounts() const {
    return accounts_;
  }

  static Account* getTopLevel();
  static Account* createCurrencyTradingAccount(const std::string& _currency);
  static Account* createSecurityTradingAccount(int _idSecurity);
  static Account* getFromPath(const std::string& _path);

  static bool accountIsCurrencyTrading(int _id);

  static bool possibleType(int _childType, int _parentType);
  static std::vector<int> possibleTypes(int _parentType);
  static int generalType(int _type);
  static std::string typeToString(int _type);
  static std::string typeToIcon(int _type);
  static void debitCreditLabelsFor(int _type, std::string& _debit,
                                   std::string& _credit);
  static bool negativeDebits(int _type);
  static int accountTypeWeight(int _type);
  static AccountClassification classificationForType(int _type);

  static bool matchesFlags(const Account* _a, int _flags);

  static bool addCustomType(CustomType* _custom, std::string& _errorMsg);

  static const char kPathSeparator;

 protected:
  void load(QXmlStreamReader& _reader) override;
  void afterLoad() override;
  void save(QXmlStreamWriter& _writer) const override;
  void unload() override;

 private:
  Account();

  static void setupTopLevel();
  static void setupAccount(Account* a);
  static void checkRootTradingAccount();

  void rec_load(QXmlStreamReader& _reader, Account* _parent);
  void rec_save(QXmlStreamWriter& _writer) const;

  int type_;

  std::string name_;
  std::string mainCurrency_;
  std::vector<std::string> secondaryCurrencies_;
  std::string code_;
  std::string note_;
  bool isPlaceholder_;
  bool isOpen_;

  int idInstitution_;
  int idSecurity_;
  int idPicture_;

  std::vector<Account*> children_;
  Account* parent_;

  Ledger* ledger_;
  Properties* properties_;
  Account* topTradingAccount_;

  static Account* topLevel_;
  static int nextId_;
  static std::unordered_map<int, Account*> accounts_;
  static std::unordered_map<int, CustomType*> customTypes_;
};

}  // namespace

#endif  // ACCOUNT_H
