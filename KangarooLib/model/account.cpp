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

#include "account.h"

#include <QStack>
#include <QXmlStreamReader>

#include "../controller/io.h"
#include "../ui/core.h"
#include "currency.h"
#include "institution.h"
#include "ledger.h"
#include "modelexception.h"
#include "picturemanager.h"
#include "pricemanager.h"
#include "schedule.h"
#include "security.h"

namespace KLib {
int Account::m_nextId = 1;
Account* Account::m_topLevel = nullptr;
Account::Key Account::publicKey = Account::Key(false);
const char Account::PATH_SEPARATOR = ':';
QHash<int, Account*> Account::m_accounts = QHash<int, Account*>();
QHash<int, CustomType*> Account::m_customTypes = QHash<int, CustomType*>();
const int CustomType::START_ID_CUSTOM = 50;

Account::Account()
    : m_type(Constants::NO_ID),
      m_isPlaceholder(false),
      m_isOpen(true),
      m_idInstitution(Constants::NO_ID),
      m_idSecurity(Constants::NO_ID),
      m_idDefaultDividendAccount(Constants::NO_ID),
      m_parent(nullptr),
      m_ledger(nullptr),
      m_properties(new Properties()),
      m_topTradingAccount(nullptr) {
  connect(m_properties, SIGNAL(modified()), this, SIGNAL(modified()));
}

Account::~Account() {
  for (Account* a : m_children) {
    delete a;
  }

  m_ledger->deleteLater();
  m_properties->deleteLater();
}

Account* Account::getFromPath(const QString& _path) {
  QStringList path = _path.split(Account::PATH_SEPARATOR, Qt::KeepEmptyParts);
  Account* cur = nullptr;

  if (path.length() > 0) {
    int i = 0;
    cur = m_topLevel;

    if (path[0] == m_topLevel->name())  // Skip the top level
    {
      i = 1;
    }

    for (; i < path.count(); ++i) {
      bool found = false;

      for (Account* a : cur->m_children) {
        if (a->name() == path[i]) {
          cur = a;
          found = true;
          break;
        }
      }

      if (!found) {
        return nullptr;
      }
    }
  }

  return cur;
}

Account* Account::getTopLevel() {
  if (!m_topLevel) {
    setupTopLevel();
  }
  return m_topLevel;
}

Account* Account::parent() const { return hasParent() ? m_parent : nullptr; }

bool Account::hasParent() const { return m_parent != nullptr; }

bool Account::isAncestorOf(const Account* _account) const {
  return _account->isDescendantOf(this);
}

bool Account::isDescendantOf(const Account* _account) const {
  Account* p = m_parent;

  while (p && p != _account) {
    p = p->m_parent;
  }

  return p;
}

void Account::moveToParent(Account* _parent) {
  if (!_parent) {
    ModelException::throwException(tr("New parent is null."), this);
  } else if (m_id == m_topLevel->id()) {
    ModelException::throwException(tr("Cannot move the top level account."),
                                   this);
  } else if (!possibleType(m_type, _parent->type())) {
    ModelException::throwException(
        tr("The account type is incompatible with the parent type."), this);
  }

  if (_parent->id() != m_parent->id()) {
    for (auto i = m_parent->m_children.begin(); i != m_parent->m_children.end();
         ++i) {
      if ((*i)->id() == m_id) {
        m_parent->m_children.erase(i);
        emit accountRemoved(this);
        break;
      }
    }

    m_parent = _parent;
    _parent->m_children.push_back(this);
    emit accountAdded(this);
    emit m_topLevel->accountModified(this);
  }
}

void Account::moveToParent(Account* _parent, int _newType) {
  if (!_parent) {
    ModelException::throwException(tr("New parent is null."), this);
    return;
  } else if (m_id == m_topLevel->id()) {
    ModelException::throwException(tr("Cannot move the top level account."),
                                   this);
    return;
  } else if (!possibleType(_newType, _parent->type())) {
    ModelException::throwException(
        tr("The account type is incompatible with the parent type."), this);
    return;
  }

  // Check if all children are OK
  for (Account* c : m_children) {
    if (!possibleType(c->type(), _newType)) {
      ModelException::throwException(
          tr("At least one child is incompatible with the new type."), this);
      return;
    }
  }

  if (_parent->id() == m_parent->id()) {
    return;
  } else if ((_newType == AccountType::INCOME &&
              m_type == AccountType::EXPENSE) ||
             (_newType == AccountType::EXPENSE &&
              m_type == AccountType::INCOME) ||
             (_newType == AccountType::EXPENSE &&
              m_type == AccountType::EQUITY) ||
             (_newType == AccountType::INCOME &&
              m_type == AccountType::EQUITY) ||
             (_newType == AccountType::EQUITY &&
              m_type == AccountType::INCOME) ||
             (_newType == AccountType::EQUITY &&
              m_type == AccountType::EXPENSE) ||
             (_newType == AccountType::ASSET &&
              m_type == AccountType::LIABILITY) ||
             (_newType == AccountType::LIABILITY &&
              m_type == AccountType::ASSET)) {
    m_type = _newType;

    //            if (!onHoldToModify())
    //                emit accountModified(this);

    for (auto i = m_parent->m_children.begin(); i != m_parent->m_children.end();
         ++i) {
      if ((*i)->id() == m_id) {
        m_parent->m_children.erase(i);
        emit accountRemoved(this);
        break;
      }
    }

    m_parent = _parent;
    _parent->m_children.push_back(this);
    emit accountAdded(this);
    emit m_topLevel->accountModified(this);
  }
}

Account* Account::getAccount(int _id) {
  auto it = m_topLevel->m_accounts.find(_id);
  if (it == m_topLevel->m_accounts.end()) {
    ModelException::throwException(tr("No such account."), m_topLevel);
    return nullptr;
  }

  return it.value();
}

Account* Account::getChild(int _id) const {
  auto i = m_children.begin();
  while (i != m_children.end()) {
    if ((*i)->id() == _id) {
      return *i;
    } else {
      Account* c = (*i)->getChild(_id);

      if (c != 0) {
        return c;
      }
    }

    ++i;
  }

  if (id() == getTopLevel()->id()) {
    ModelException::throwException(tr("No such account."), this);
    return nullptr;
  } else {
    return nullptr;
  }
}

QString Account::getPath(int _id, int _upToHeight) const {
  Account* a = m_accounts.value(_id);

  if (!a) return "";

  QStack<QString> stack;
  QString str;

  if (_upToHeight == 0) {
    _upToHeight = INT_MAX;
  }

  // Go up until we reach the top of the tree, or the max height requested,
  // whichever is higher.
  while (
      a->hasParent() &&
      _upToHeight >
          0)  // This way, top level won't be there as it doesn't have a parent.
  {
    stack.push(a->name());
    a = a->parent();
    --_upToHeight;
  }

  if (stack.count() > 0) {
    while (stack.count() > 1) {
      str += stack.pop() + ":";
    }

    str += stack.pop();
    return str;
  } else {
    return "";
  }
}

Account* Account::getChild(const QString& _code) const {
  auto i = m_children.begin();
  while (i != m_children.end()) {
    if ((*i)->code() == _code) {
      return *i;
    } else {
      Account* c = (*i)->getChild(_code);

      if (c != 0) {
        return c;
      }
    }

    ++i;
  }

  if (id() == getTopLevel()->id()) {
    ModelException::throwException(tr("No such account."), this);
  }
  return nullptr;
}

Account* Account::addChild(const QString& _name, int _type,
                           const QString& _currency, int _idSecurity,
                           bool _placeholder, int _idInstitution,
                           const QString& _code, const QString& _note,
                           const Key& _k) {
  if (!_k.priv &&
      (_type == AccountType::TRADING || _type == AccountType::TOPLEVEL)) {
    ModelException::throwException(
        tr("This type of account cannot be added manually."), this);
  }

  if (!possibleType(_type, type())) {
    ModelException::throwException(tr("This type of account is not supported "
                                      "by the parent of this account."),
                                   this);
  }

  if (_type == AccountType::INVESTMENT ||
      (_type == AccountType::TRADING && _currency.isEmpty())) {
    SecurityManager::instance()->get(_idSecurity);
  } else {
    CurrencyManager::instance()->get(_currency);
  }

  if (_idInstitution != Constants::NO_ID) {
    InstitutionManager::instance()->get(_idInstitution);
  }

  Account* acc = new Account();
  acc->m_id = m_nextId++;
  acc->m_name = _name;
  acc->m_type = _type;
  acc->m_mainCurrency = !_currency.isEmpty() ? _currency : "";
  acc->m_idSecurity = _currency.isEmpty() ? _idSecurity : Constants::NO_ID;
  acc->m_isPlaceholder = _placeholder;
  acc->m_idInstitution = _idInstitution;
  acc->m_code = _code;
  acc->m_note = _note;
  acc->m_parent = this;

  // Check if custom
  if (m_customTypes.contains(_type)) {
    QString err;
    if (!m_customTypes[_type]->newValidation(acc, err)) {
      delete acc;
      ModelException::throwException(err, this);
    }
  }

  setupAccount(acc);

  m_children.push_back(acc);
  m_accounts[acc->m_id] = acc;

  emit accountAdded(acc);
  emit m_topLevel->modified();

  return acc;
}

void Account::setupAccount(Account* a) {
  a->m_ledger = a->isPlaceholder() ? nullptr : new Ledger(a);

  connect(a->properties(), SIGNAL(modified()), a, SLOT(onPropertiesModified()));
  connect(a, SIGNAL(accountAdded(KLib::Account*)), m_topLevel,
          SIGNAL(accountAdded(KLib::Account*)));
  connect(a, SIGNAL(accountModified(KLib::Account*)), m_topLevel,
          SIGNAL(accountModified(KLib::Account*)));
  connect(a, SIGNAL(accountRemoved(KLib::Account*)), m_topLevel,
          SIGNAL(accountRemoved(KLib::Account*)));
  LedgerManager::instance()->addAccount(a);
}

bool Account::removeChild(int _id, const Key& _k) {
  for (auto i = m_children.begin(); i != m_children.end(); ++i) {
    if ((*i)->id() == _id) {
      Account* del = *i;

      if (del->m_type == AccountType::TRADING && !_k.priv) {
        ModelException::throwException(
            tr("Currency trading accounts cannot be removed."), this);
        break;
      } else if (del->childCount() > 0 && !_k.priv) {
        ModelException::throwException(
            tr("Accounts with children cannot be removed."), this);
        break;
      }

      // Check if custom
      if (del->type() >= CustomType::START_ID_CUSTOM &&
          m_customTypes.contains(del->type())) {
        QString err;
        if (!m_customTypes[del->type()]->deleteValidation(del, err)) {
          ModelException::throwException(err, this);
          break;
        }
      }

      LedgerManager::instance()->removeAccount(del);

      emit accountRemoved(del);
      emit m_topLevel->modified();

      m_children.erase(i);
      m_accounts.remove(_id);
      del->deleteLater();

      // Remove schedules with this account if some exist
      ScheduleManager::instance()->removeSchedulesForAccount(_id);

      return true;
    }
  }

  return false;
}

bool Account::canBeRemoved() const {
  QString err;
  return !(m_type == AccountType::TRADING || m_type == AccountType::TOPLEVEL ||
           childCount() || (!isPlaceholder() && m_ledger->count()) ||
           (m_type >= CustomType::START_ID_CUSTOM &&
            m_customTypes.contains(m_type) &&
            !m_customTypes[m_type]->deleteValidation(this, err)));
}

QSet<QString> Account::allCurrencies() const {
  QSet<QString> cur = m_secondaryCurrencies;
  cur.insert(m_mainCurrency);
  return cur;
}

void Account::setType(int _type) {
  if (_type == m_type) return;

  if (_type == AccountType::TOPLEVEL || _type == AccountType::TRADING) {
    ModelException::throwException(
        tr("This type of account cannot be added manually."), this);
  } else if (m_type == AccountType::INVESTMENT) {
    ModelException::throwException(
        tr("Cannot change the type of an investment account."), this);
  } else if (_type == AccountType::INVESTMENT) {
    ModelException::throwException(
        tr("An existing account cannot be made an investment account."), this);
  }

  if ((m_type != AccountType::TOPLEVEL && m_type != AccountType::TRADING) &&
      possibleType(_type, m_parent->type())) {
    // Check if OK with all children
    for (Account* child : m_children) {
      if (!possibleType(child->type(), _type)) {
        ModelException::throwException(
            tr("This type of account is not supported by at least child of "
               "this account."),
            this);
      }
    }

    m_type = _type;

    if (!onHoldToModify()) emit accountModified(this);
  } else {
    ModelException::throwException(tr("This type of account is not supported "
                                      "by the parent of this account."),
                                   this);
  }
}

void Account::setName(const QString& _name) {
  m_name = _name;
  if (!onHoldToModify()) emit accountModified(this);
}

void Account::setCurrency(const QString& _currency) {
  if (_currency == m_mainCurrency) return;

  // See if it exists
  CurrencyManager::instance()->get(_currency);

  if (m_type == AccountType::TRADING) {
    ModelException::throwException(
        tr("Cannot change the currency of a currency trading account."), this);
  } else if (m_type == AccountType::INVESTMENT) {
    ModelException::throwException(
        tr("Cannot set the currency of an investment account."), this);
  } else if (!isPlaceholder() && m_ledger->count()) {
    // Check if part of the secondary currencies
    if (m_secondaryCurrencies.contains(_currency)) {
      m_secondaryCurrencies.insert(m_mainCurrency);
      m_secondaryCurrencies.remove(_currency);
      m_mainCurrency = _currency;
    } else {
      ModelException::throwException(
          tr("Cannot change the currency of an active account."), this);
    }
  } else {
    m_secondaryCurrencies.remove(_currency);
    m_mainCurrency = _currency;
  }

  if (!onHoldToModify()) emit accountModified(this);
}

void Account::setSecondaryCurrencies(const QSet<QString>& _currencies) {
  if (!_currencies.contains(m_secondaryCurrencies) && !m_isPlaceholder) {
    // We removed some currencies!
    QSet<QString> removed = m_secondaryCurrencies.subtract(_currencies);
    removed.remove(m_mainCurrency);
    QSet<QString> used = m_ledger->currenciesUsed();

    if (used.intersects(removed)) {
      ModelException::throwException(
          tr("Cannot remove secondary currencies that are in use."), this);
    }
  }

  // Everything is ok!
  m_secondaryCurrencies = _currencies;

  // Remove the main currency in case it's there...
  m_secondaryCurrencies.remove(m_mainCurrency);
}

void Account::setCode(const QString& _code) {
  m_code = _code;

  if (!onHoldToModify()) emit accountModified(this);
}

void Account::setNote(const QString& _note) {
  m_note = _note;

  if (!onHoldToModify()) emit accountModified(this);
}

void Account::setOpen(bool _open) {
  if (_open == m_isOpen) return;

  if (!_open) {
    if (m_type == AccountType::TRADING) {
      ModelException::throwException(
          tr("Cannot close a currency trading account."), this);
    } else if (m_type == AccountType::TOPLEVEL) {
      ModelException::throwException(tr("Cannot close the top level account."),
                                     this);
    } else if (balance() != 0) {
      ModelException::throwException(
          tr("Cannot close an account with an outstanding balance."), this);
    } else  // Check if all children are closed
    {
      for (Account* c : m_children) {
        if (c->isOpen()) {
          ModelException::throwException(
              tr("Cannot close an account that has open subaccounts"), this);
        }
      }
    }
  }

  m_isOpen = _open;

  if (!_open) {
    // Remove schedules with this account if some exist
    ScheduleManager::instance()->removeSchedulesForAccount(m_id);
  }

  if (!onHoldToModify()) emit accountModified(this);
}

bool Account::canBeClosed() const {
  // Check if all children are closed
  for (Account* c : m_children) {
    if (c->isOpen()) {
      return false;
    }
  }

  // Other validations
  return m_isOpen && m_type != AccountType::TRADING &&
         m_type != AccountType::TOPLEVEL && balance() == 0;
}

void Account::setIsPlaceholder(bool _isPlaceholder) {
  if (m_isPlaceholder == _isPlaceholder) {
    return;
  }

  if (!isPlaceholder() && m_ledger->count()) {
    ModelException::throwException(tr("Impossible to make this account a "
                                      "placeholder, as it has transactions."),
                                   this);
  } else if (!isPlaceholder() && m_type == AccountType::TRADING) {
    ModelException::throwException(
        tr("Currency trading accounts cannot be placeholders."), this);
  } else if (m_type == AccountType::TOPLEVEL || this == m_topTradingAccount) {
    ModelException::throwException(
        tr("The placeholder status cannot be removed for this account."), this);
  }

  if (!_isPlaceholder) {
    m_isPlaceholder = _isPlaceholder;  // Must be done >>before<< LedgerManager
    m_ledger = new Ledger(this);
    LedgerManager::instance()->addAccount(this);
  } else {
    LedgerManager::instance()->removeAccount(this);
    m_ledger->deleteLater();
    m_ledger = nullptr;
    m_isPlaceholder = _isPlaceholder;  // Must be done >>after<< LedgerManager

    // Remove schedules with this account if some exist
    ScheduleManager::instance()->removeSchedulesForAccount(m_id);
  }

  if (!onHoldToModify()) emit accountModified(this);
}

void Account::setIdInstitution(int _id) {
  if (_id == m_idInstitution) return;

  if (m_type == AccountType::TRADING) {
    ModelException::throwException(
        tr("Cannot change institution of a currency trading account."), this);
  } else if (m_type == AccountType::TOPLEVEL) {
    ModelException::throwException(
        tr("Cannot change institution of a top level account."), this);
  }

  if (_id != Constants::NO_ID) {
    // Check that it exists
    InstitutionManager::instance()->get(_id);
  }

  m_idInstitution = _id;

  if (!onHoldToModify()) emit accountModified(this);
}

void Account::setIdPicture(int _id) {
  if (_id == m_idPicture) return;

  if (_id != Constants::NO_ID) {
    // Check that it exists
    PictureManager::instance()->get(_id);
  }

  m_idPicture = _id;

  if (!onHoldToModify()) emit accountModified(this);
}

QPixmap Account::defaultPicture(bool _thumbnail) const {
  try {
    int id = m_idPicture;

    if (id == Constants::NO_ID) {
      if ((m_type == AccountType::INVESTMENT ||
           (m_type == AccountType::TRADING &&
            m_idSecurity != Constants::NO_ID)) &&
          SecurityManager::instance()
              ->get(m_idSecurity)
              ->defaultToSecurityPicture() &&
          SecurityManager::instance()->get(m_idSecurity)->idPicture() !=
              Constants::NO_ID) {
        id = SecurityManager::instance()->get(m_idSecurity)->idPicture();
      } else {
        return Core::pixmap(typeToIcon(m_type));
      }
    }

    if (_thumbnail) {
      return PictureManager::instance()->get(id)->thumbnail;
    } else {
      return PictureManager::instance()->get(id)->picture;
    }
  } catch (ModelException) {
    return Core::pixmap(typeToIcon(m_type));
  }
}

void Account::setIdSecurity(int _id) {
  if (_id == m_idSecurity) return;

  if (m_type != AccountType::INVESTMENT) {
    ModelException::throwException(
        tr("Cannot set the security of a non-investment account."), this);
  } else if (!isPlaceholder() && m_ledger->count()) {
    ModelException::throwException(
        tr("Cannot change the security of an active account."), this);
  }

  if (_id != Constants::NO_ID) {
    // Check that it exists
    SecurityManager::instance()->get(_id);
  }

  m_idSecurity = _id;

  if (!onHoldToModify()) emit accountModified(this);
}

void Account::setIdDefaultDividendAccount(int _id) {
  if (_id == m_idDefaultDividendAccount) return;

  if (m_type != AccountType::INVESTMENT && m_type != AccountType::BROKERAGE) {
    ModelException::throwException(tr("Cannot set the default dividend account "
                                      "of a non-investment account."),
                                   this);
  }

  if (_id != Constants::NO_ID) {
    // Check that it exists
    Account::getAccount(_id);
  }

  m_idDefaultDividendAccount = _id;

  if (!onHoldToModify()) emit accountModified(this);
}

Amount Account::balance() const { return balanceBetween(QDate(), QDate()); }

Amount Account::balanceAt(const QDate& _date) const {
  return balanceBetween(QDate(), _date);
}

Amount Account::treeValueAt(const QDate& _date) const {
  return treeValueBetween(QDate(), _date);
}

Amount Account::balanceToday() const {
  return balanceBetween(QDate(), QDate::currentDate());
}

Amount Account::treeValueToday() const {
  return treeValueBetween(QDate(), QDate::currentDate());
}

KLib::Amount Account::balanceBetween(const QDate& _start,
                                     const QDate& _end) const {
  if (m_ledger) {
    Amount bal = m_ledger->balanceBetween(_start, _end);

    if (negativeDebits(m_type)) {
      bal *= -1;
    }

    return bal;
  } else {
    return 0;
  }
}

KLib::Amount Account::treeValueBetween(const QDate& _start,
                                       const QDate& _end) const {
  Amount total = 0;

  if (m_mainCurrency.isEmpty())  // Security
  {
    total = balanceBetween(_start, _end) *
            PriceManager::instance()->rate(m_idSecurity,
                                           m_topLevel->mainCurrency(), _end);
  } else if (m_mainCurrency !=
             m_topLevel->m_mainCurrency)  // Not default currency
  {
    total = balanceBetween(_start, _end) *
            PriceManager::instance()->rate(m_mainCurrency,
                                           m_topLevel->mainCurrency(), _end);
  } else  // Default currency
  {
    total = balanceBetween(_start, _end);
  }

  for (Account* c : m_children) {
    total += c->treeValueBetween(_start, _end);
  }

  return total;
}

Amount Account::treeValue() const { return treeValueBetween(QDate(), QDate()); }

QString Account::formatAmount(const Amount& _amount) const {
  if (m_mainCurrency.isEmpty()) {
    return QString::number(_amount.toDouble());
    //            return QString("%1
    //            %2").arg(QString::number(_amount.toDouble()))
    //                                               .arg(SecurityManager::instance()->get(m_idSecurity)->symbol());
  } else {
    return CurrencyManager::instance()
        ->get(m_mainCurrency)
        ->formatAmount(_amount);
  }
}

void Account::doneHoldToModify() {
  emit accountModified(this);
  IStored::doneHoldToModify();
}

void Account::onPropertiesModified() { emit accountModified(this); }

void Account::load(QXmlStreamReader& _reader) {
  unload();
  m_accounts.clear();

  this->rec_load(_reader, nullptr);  // this = m_topLevel
  m_accounts[m_id] = this;
}

void Account::rec_load(QXmlStreamReader& _reader, Account* _parent) {
  QXmlStreamAttributes attributes = _reader.attributes();

  m_id = IO::getAttribute("id", attributes).toInt();
  m_type = IO::getAttribute("type", attributes).toInt();
  m_name = IO::getAttribute("name", attributes);
  m_mainCurrency = IO::getAttribute("currency", attributes);
  m_code = IO::getOptAttribute("code", attributes);
  m_note = IO::getOptAttribute("note", attributes);
  m_isPlaceholder = IO::getAttribute("placeholder", attributes) == "true";
  m_isOpen = IO::getAttribute("open", attributes) == "true";
  m_idInstitution =
      IO::getOptAttribute("institution", attributes, Constants::NO_ID).toInt();
  m_idPicture =
      IO::getOptAttribute("picture", attributes, Constants::NO_ID).toInt();
  m_idSecurity =
      IO::getOptAttribute("security", attributes, Constants::NO_ID).toInt();
  m_idDefaultDividendAccount =
      IO::getOptAttribute("dividendaccount", attributes, Constants::NO_ID)
          .toInt();

  const QStringList clist =
      IO::getOptAttribute("secondarycurrencies", attributes, "")
          .split(',', Qt::SkipEmptyParts);

  m_secondaryCurrencies.clear();

  for (const QString& c : clist) {
    m_secondaryCurrencies.insert(c);
  }

  m_nextId = std::max(m_nextId, m_id + 1);

  m_parent = _parent;

  // Check if is top level currency trading
  if (_parent == m_topLevel && m_type == AccountType::TRADING) {
    m_topLevel->m_topTradingAccount = this;
  }

  if (_reader.readNextStartElement()) {
    while (!(_reader.tokenType() == QXmlStreamReader::EndElement &&
             _reader.name() == StdTags::ACCOUNT)) {
      if (_reader.tokenType() == QXmlStreamReader::StartElement &&
          _reader.name() == StdTags::ACCOUNT) {
        Account* c = new Account();
        m_children.push_back(c);

        c->rec_load(_reader, this);
        m_accounts[c->m_id] = c;

        setupAccount(c);
      } else if (_reader.tokenType() == QXmlStreamReader::StartElement &&
                 _reader.name() == StdTags::PROPERTIES) {
        m_properties->load(_reader);
      }

      _reader.readNext();
    }
  }
}

void Account::save(QXmlStreamWriter& _writer) const { this->rec_save(_writer); }

void Account::rec_save(QXmlStreamWriter& _writer) const {
  _writer.writeAttribute("id", QString::number(m_id));
  _writer.writeAttribute("type", QString::number(m_type));
  _writer.writeAttribute("name", m_name);
  _writer.writeAttribute("currency", m_mainCurrency);
  _writer.writeAttribute("secondarycurrencies",
                         m_secondaryCurrencies.values().join(','));
  _writer.writeAttribute("code", m_code);
  _writer.writeAttribute("note", m_note);
  _writer.writeAttribute("placeholder", m_isPlaceholder ? "true" : "false");
  _writer.writeAttribute("open", m_isOpen ? "true" : "false");
  _writer.writeAttribute("institution", QString::number(m_idInstitution));
  _writer.writeAttribute("picture", QString::number(m_idPicture));
  _writer.writeAttribute("security", QString::number(m_idSecurity));
  _writer.writeAttribute("dividendaccount",
                         QString::number(m_idDefaultDividendAccount));
  m_properties->save(_writer);

  for (Account* a : m_children) {
    _writer.writeStartElement(StdTags::ACCOUNT);
    a->rec_save(_writer);
    _writer.writeEndElement();
  }
}

void Account::unload() {
  for (Account* c : m_topLevel->getChildren()) {
    delete c;
  }

  m_topLevel->m_children.clear();
  m_topLevel->m_topTradingAccount = nullptr;
  m_accounts.clear();
  m_accounts[m_topLevel->id()] = m_topLevel;

  LedgerManager::instance()->unload();
}

void Account::afterLoad() {
  // Add the currency trading accounts to all currencies
  if (m_topLevel->m_topTradingAccount) {
    for (Account* c : m_topLevel->m_topTradingAccount->m_children) {
      if (!c->mainCurrency().isEmpty()) {
        CurrencyManager::instance()->get(c->mainCurrency())->m_trading = c;
      } else {
        SecurityManager::instance()->get(c->idSecurity())->m_trading = c;
      }
    }
  }

  LedgerManager::instance()->load();
}

void Account::setupTopLevel() {
  m_topLevel = new Account();

  m_topLevel->m_id = 0;
  m_topLevel->m_type = AccountType::TOPLEVEL;
  m_topLevel->m_name = typeToString(AccountType::TOPLEVEL);
  m_topLevel->m_isPlaceholder = true;
  m_topLevel->m_mainCurrency = Constants::DEFAULT_CURRENCY_CODE;
  m_accounts[m_topLevel->m_id] = m_topLevel;

  connect(m_topLevel, SIGNAL(accountModified(KLib::Account*)), m_topLevel,
          SIGNAL(modified()));

  LedgerManager::m_instance = new LedgerManager();
}

Account* Account::createCurrencyTradingAccount(const QString& _currency) {
  // See if the account exists (will throw an exception if the currency does
  // not)
  Account* trading = CurrencyManager::instance()->get(_currency)->m_trading;

  if (!trading) {
    // Check if the currency trading top level exists
    checkRootTradingAccount();

    // Create the currency trading account
    trading = m_topLevel->m_topTradingAccount->addChild(
        _currency, AccountType::TRADING, _currency, Constants::NO_ID, false,
        Constants::NO_ID, "", "", Key(true));

    CurrencyManager::instance()->get(_currency)->m_trading = trading;
  }

  return trading;
}

Account* Account::createSecurityTradingAccount(int _idSecurity) {
  // See if it the account exists (will throw an exception if the currency does
  // not)
  Security* security = SecurityManager::instance()->get(_idSecurity);
  Account* trading = security->m_trading;

  if (!trading) {
    // Check if the currency trading top level exists
    checkRootTradingAccount();

    // Create the currency trading account
    trading = m_topLevel->m_topTradingAccount->addChild(
        security->name(), AccountType::TRADING, "", _idSecurity, false,
        Constants::NO_ID, "", "", Key(true));

    security->m_trading = trading;
  }

  return trading;
}

void Account::checkRootTradingAccount() {
  if (!m_topLevel->m_topTradingAccount) {
    m_topLevel->m_topTradingAccount = m_topLevel->addChild(
        tr("Trading"), AccountType::TRADING, m_topLevel->mainCurrency(),
        Constants::NO_ID, true, Constants::NO_ID, "", "", Key(true));
  }
}

bool Account::accountIsCurrencyTrading(int _id) {
  if (m_topLevel->m_topTradingAccount != nullptr) {
    for (Account* c : m_topLevel->m_topTradingAccount->m_children) {
      if (c->id() == _id) {
        return true;
      }
    }
  }

  return false;
}

bool Account::possibleType(int _childType, int _parentType) {
  if (m_customTypes.contains(_childType)) {
    return m_customTypes[_childType]->parents.contains(_parentType);
  } else {
    switch (_parentType) {
      case AccountType::TOPLEVEL:
        switch (_childType) {
          case AccountType::ASSET:
          case AccountType::LIABILITY:
          case AccountType::EQUITY:
          case AccountType::INCOME:
          case AccountType::EXPENSE:
          case AccountType::TRADING:
            return true;
          default:
            return false;
        }
      case AccountType::ASSET:
      case AccountType::CASH:
      case AccountType::CHECKING:
      case AccountType::SAVINGS:
      case AccountType::BROKERAGE:
      case AccountType::DEPOSIT:
      case AccountType::PREPAIDCARD:
      case AccountType::PROPERTY:
        switch (_childType) {
          case AccountType::ASSET:
          case AccountType::CASH:
          case AccountType::CHECKING:
          case AccountType::SAVINGS:
          case AccountType::BROKERAGE:
          case AccountType::INVESTMENT:
          case AccountType::DEPOSIT:
          case AccountType::PREPAIDCARD:
          case AccountType::PROPERTY:
            return true;
          default:
            return false;
        }
      case AccountType::INVESTMENT:
        return (_childType == AccountType::INVESTMENT);
      case AccountType::LIABILITY:
      case AccountType::CREDITCARD:
        switch (_childType) {
          case AccountType::LIABILITY:
          case AccountType::CREDITCARD:
            return true;
          default:
            return false;
        }
      case AccountType::EQUITY:
        return (_childType == AccountType::EQUITY);
      case AccountType::INCOME:
        return (_childType == AccountType::INCOME);
      case AccountType::EXPENSE:
        return (_childType == AccountType::EXPENSE);
      case AccountType::TRADING:
        return (_childType == AccountType::TRADING);
    }

    return false;
  }
}

QList<int> Account::possibleTypes(int _parentType) {
  if (m_customTypes.contains(_parentType)) {
    return m_customTypes[_parentType]->children.values();
  } else {
    QList<int> types;

    switch (_parentType) {
      case AccountType::TOPLEVEL:
        types << AccountType::ASSET << AccountType::LIABILITY
              << AccountType::EQUITY << AccountType::INCOME
              << AccountType::EXPENSE << AccountType::TRADING;
        break;
      case AccountType::ASSET:
      case AccountType::CASH:
      case AccountType::CHECKING:
      case AccountType::SAVINGS:
      case AccountType::BROKERAGE:
      case AccountType::DEPOSIT:
      case AccountType::PREPAIDCARD:
      case AccountType::PROPERTY:
        types << AccountType::ASSET << AccountType::CASH
              << AccountType::CHECKING << AccountType::SAVINGS
              << AccountType::BROKERAGE << AccountType::INVESTMENT
              << AccountType::DEPOSIT << AccountType::PREPAIDCARD
              << AccountType::PROPERTY;
        break;
      case AccountType::INVESTMENT:
        types << AccountType::INVESTMENT;
        break;
      case AccountType::LIABILITY:
      case AccountType::CREDITCARD:
        types << AccountType::LIABILITY << AccountType::CREDITCARD;
        break;
      case AccountType::EQUITY:
        types << AccountType::EQUITY;
        break;
      case AccountType::INCOME:
        types << AccountType::INCOME;
        break;
      case AccountType::EXPENSE:
        types << AccountType::EXPENSE;
        break;
      case AccountType::TRADING:
        types << AccountType::TRADING;
        break;
      default:
        break;
    }

    for (CustomType* c : qAsConst(m_customTypes)) {
      if (c->parents.contains(_parentType)) {
        types << c->id;
      }
    }

    return types;
  }
}

int Account::generalType(int _type) {
  if (m_customTypes.contains(_type)) {
    return m_customTypes[_type]->generalType;
  } else {
    switch (_type) {
      case AccountType::TOPLEVEL:
        return AccountType::TOPLEVEL;

      case AccountType::ASSET:
        return AccountType::ASSET;

      case AccountType::LIABILITY:
        return AccountType::LIABILITY;

      case AccountType::EQUITY:
        return AccountType::EQUITY;

      case AccountType::INCOME:
        return AccountType::INCOME;

      case AccountType::EXPENSE:
        return AccountType::EXPENSE;

      case AccountType::CASH:
        return AccountType::ASSET;

      case AccountType::CHECKING:
        return AccountType::ASSET;

      case AccountType::SAVINGS:
        return AccountType::ASSET;

      case AccountType::BROKERAGE:
        return AccountType::ASSET;

      case AccountType::INVESTMENT:
        return AccountType::ASSET;

      case AccountType::DEPOSIT:
        return AccountType::ASSET;

      case AccountType::PREPAIDCARD:
        return AccountType::ASSET;

      case AccountType::PROPERTY:
        return AccountType::ASSET;

      case AccountType::CREDITCARD:
        return AccountType::LIABILITY;

      case AccountType::TRADING:
        return AccountType::TRADING;
    }

    return _type;
  }
}

QString Account::typeToString(int _type) {
  if (m_customTypes.contains(_type)) {
    return m_customTypes[_type]->name;
  } else {
    switch (_type) {
      case AccountType::TOPLEVEL:
        return tr("Top Level");

      case AccountType::ASSET:
        return tr("Asset");

      case AccountType::LIABILITY:
        return tr("Liability");

      case AccountType::EQUITY:
        return tr("Equity");

      case AccountType::INCOME:
        return tr("Income");

      case AccountType::EXPENSE:
        return tr("Expense");

      case AccountType::CASH:
        return tr("Cash");

      case AccountType::CHECKING:
        return tr("Checking");

      case AccountType::SAVINGS:
        return tr("Savings");

      case AccountType::BROKERAGE:
        return tr("Brokerage");

      case AccountType::INVESTMENT:
        return tr("Investment");

      case AccountType::DEPOSIT:
        return tr("Deposit");

      case AccountType::PREPAIDCARD:
        return tr("Prepaid Card");

      case AccountType::PROPERTY:
        return tr("Property");

      case AccountType::CREDITCARD:
        return tr("Credit Card");

      case AccountType::TRADING:
        return tr("Trading");
    }

    return QString();
  }
}

QString Account::typeToIcon(int _type) {
  if (m_customTypes.contains(_type)) {
    return m_customTypes[_type]->icon;
  } else {
    switch (_type) {
      case AccountType::CASH:
        return "cash";

      case AccountType::CHECKING:
        return "bank-account";

      case AccountType::BROKERAGE:
        return "brokerage-account";

      case AccountType::SAVINGS:
        return "savings-account";

      case AccountType::CREDITCARD:
      case AccountType::PREPAIDCARD:
        return "credit-card-account";

      case AccountType::INVESTMENT:
        return "investment-account";

      case AccountType::INCOME:
        return "income-account";

      case AccountType::EXPENSE:
        return "expense-account";

      case AccountType::LIABILITY:
        return "liability-account";

      default:
        return "bank";
    }
  }
}

void Account::debitCreditLabelsFor(int _type, QString& _debit,
                                   QString& _credit) {
  if (_type < CustomType::START_ID_CUSTOM) {
    switch (_type) {
      case AccountType::ASSET:
      case AccountType::DEPOSIT:
      case AccountType::CASH:
      case AccountType::PREPAIDCARD:
      case AccountType::TRADING:
      case AccountType::PROPERTY:
        _debit = tr("Increase");
        _credit = tr("Decrease");
        break;

      case AccountType::CHECKING:
        _debit = tr("Deposit");
        _credit = tr("Withdrawal");
        break;

      case AccountType::INVESTMENT:
        _debit = tr("Buy");
        _credit = tr("Sell");
        break;

      case AccountType::LIABILITY:
      case AccountType::EQUITY:
        _debit = tr("Decrease");
        _credit = tr("Increase");
        break;

      case AccountType::CREDITCARD:
        _debit = tr("Payment");
        _credit = tr("Charge");
        break;

      case AccountType::EXPENSE:
        _debit = tr("Expense");
        _credit = tr("Rebate");
        break;

      case AccountType::INCOME:
        _debit = tr("Charge");
        _credit = tr("Income");
        break;

      default:
        _debit = tr("Debit");
        _credit = tr("Credit");
        break;
    }
  } else if (m_customTypes.contains(_type)) {
    _debit = m_customTypes[_type]->customDebit;
    _credit = m_customTypes[_type]->customCredit;
  }
}

bool Account::negativeDebits(int _type) {
  if (_type < CustomType::START_ID_CUSTOM) {
    switch (generalType(_type)) {
      case AccountType::LIABILITY:
      case AccountType::INCOME:
      case AccountType::EQUITY:
        return true;
      default:
        return false;
    }
  } else if (m_customTypes.contains(_type)) {
    return m_customTypes[_type]->negativeDebits;
  } else {
    return false;
  }
}

int Account::accountTypeWeight(int _type) {
  if (_type < CustomType::START_ID_CUSTOM) {
    switch (_type) {
      case AccountType::TOPLEVEL:
        return INT_MIN;

      case AccountType::ASSET:
        return -1000;

      case AccountType::LIABILITY:
        return -500;

      case AccountType::EQUITY:
        return 0;

      case AccountType::INCOME:
        return 500;

      case AccountType::EXPENSE:
        return 1000;

      case AccountType::TRADING:
        return 1500;

      case AccountType::CHECKING:
        return -900;

      case AccountType::BROKERAGE:
        return -850;

      case AccountType::SAVINGS:
        return -800;

      case AccountType::CASH:
        return -750;

      case AccountType::PROPERTY:
        return -720;

      case AccountType::INVESTMENT:
        return -700;

      case AccountType::PREPAIDCARD:
        return -650;

      case AccountType::DEPOSIT:
        return -600;

      case AccountType::CREDITCARD:
        return -450;
    }
  } else if (m_customTypes.contains(_type)) {
    return m_customTypes[_type]->weight;
  }

  return INT_MAX;  // Sink as much as possible!
}

AccountClassification Account::classificationForType(int _type) {
  if (_type < CustomType::START_ID_CUSTOM) {
    switch (_type) {
      case AccountType::CHECKING:
      case AccountType::SAVINGS:
      case AccountType::CASH:
      case AccountType::CREDITCARD:
      case AccountType::PREPAIDCARD:
        return AccountClassification::CASH_FLOW;

      case AccountType::BROKERAGE:
      case AccountType::INVESTMENT:
        return AccountClassification::INVESTING;

      case AccountType::PROPERTY:
      case AccountType::LIABILITY:
        return AccountClassification::PROPERTY_DEBT;

      case AccountType::EQUITY:
      case AccountType::ASSET:
      case AccountType::DEPOSIT:
        return AccountClassification::OTHER;

      default:
        return AccountClassification::INVALID;
    }
  } else if (m_customTypes.contains(_type)) {
    return m_customTypes[_type]->classification;
  } else {
    return AccountClassification::INVALID;
  }
}

bool Account::matchesFlags(const Account* _a, int _flags) {
  switch (_a->type()) {
    case AccountType::TOPLEVEL:
      return _flags & AccountTypeFlags::Flag_Toplevel;

    case AccountType::ASSET:
      return _flags & AccountTypeFlags::Flag_Asset;

    case AccountType::LIABILITY:
      return _flags & AccountTypeFlags::Flag_Liability;

    case AccountType::EQUITY:
      return _flags & AccountTypeFlags::Flag_Equity;

    case AccountType::INCOME:
      return _flags & AccountTypeFlags::Flag_Income;

    case AccountType::EXPENSE:
      return _flags & AccountTypeFlags::Flag_Expense;

    case AccountType::CASH:
      return _flags & AccountTypeFlags::Flag_Asset;

    case AccountType::DEPOSIT:
      return _flags & AccountTypeFlags::Flag_Asset;

    case AccountType::PREPAIDCARD:
      return _flags & AccountTypeFlags::Flag_Asset;

    case AccountType::PROPERTY:
      return _flags & AccountTypeFlags::Flag_Asset;

    case AccountType::CHECKING:
      return _flags & AccountTypeFlags::Flag_Asset;

    case AccountType::SAVINGS:
      return _flags & AccountTypeFlags::Flag_Asset;

    case AccountType::BROKERAGE:
      return _flags & AccountTypeFlags::Flag_Asset;

    case AccountType::INVESTMENT:
      return _flags & AccountTypeFlags::Flag_Investment;

    case AccountType::CREDITCARD:
      return _flags & AccountTypeFlags::Flag_Liability;

    case AccountType::TRADING:
      return _flags & AccountTypeFlags::Flag_Trading;
  }

  if (_a->type() >= CustomType::START_ID_CUSTOM) {
    // Match it with the generic type of the account.
    switch (Account::generalType(_a->type())) {
      case AccountType::ASSET:
        return _flags & AccountTypeFlags::Flag_Asset;
      case AccountType::LIABILITY:
        return _flags & AccountTypeFlags::Flag_Liability;
      case AccountType::EQUITY:
        return _flags & AccountTypeFlags::Flag_Equity;
      case AccountType::INCOME:
        return _flags & AccountTypeFlags::Flag_Income;
      case AccountType::EXPENSE:
        return _flags & AccountTypeFlags::Flag_Expense;
      case AccountType::TRADING:
        return _flags & AccountTypeFlags::Flag_Trading;
    }
  }

  // Weird unknown type, don't show that!
  return false;
}

bool Account::addCustomType(CustomType* _custom, QString& _errorMsg) {
  if (!_custom) {
    _errorMsg = tr("The type is null!");
    return false;
  } else if (_custom->id < 50) {
    _errorMsg = tr("The id must be at least 50.");
    return false;
  } else if (m_customTypes.contains(_custom->id)) {
    _errorMsg = tr("The id is already taken by another custom type");
    return false;
  } else if (_custom->name.isEmpty()) {
    _errorMsg = tr("The name is empty");
    return false;
  } else if (_custom->parents.isEmpty()) {
    _errorMsg = tr("No parents have been set");
    return false;
  } else if (_custom->customCredit.isEmpty() ||
             _custom->customDebit.isEmpty()) {
    _errorMsg = tr("The custom credit/debit are invalid");
    return false;
  } else {
    // Check the parents
    for (int i : qAsConst(_custom->parents)) {
      if (i == AccountType::TOPLEVEL ||
          (i != _custom->id && typeToString(i).isEmpty())) {
        _errorMsg = tr("At least one of the parents is invalid.");
        return false;
      }
    }

    // Check the children
    for (int i : qAsConst(_custom->children)) {
      if (i == AccountType::TOPLEVEL ||
          (i != _custom->id && typeToString(i).isEmpty())) {
        _errorMsg = tr("At least one of the children is invalid.");
        return false;
      }
    }

    if ((_custom->parents.contains(_custom->id) &&
         !_custom->children.contains(_custom->id)) ||
        (!_custom->parents.contains(_custom->id) &&
         _custom->children.contains(_custom->id))) {
      _errorMsg =
          tr("The parent-child relationship concerning this specific account "
             "is invalid.");
      return false;
    }

    // Check the general type
    switch (_custom->generalType) {
      case AccountType::ASSET:
      case AccountType::LIABILITY:
      case AccountType::EQUITY:
      case AccountType::INCOME:
      case AccountType::EXPENSE:
        break;

      default:
        _errorMsg =
            tr("The general type must be one of Asset, Liability, Equity, "
               "Income or Expense.");
        return false;
    }
  }

  m_customTypes[_custom->id] = _custom;
  return true;
}
}  // namespace KLib
