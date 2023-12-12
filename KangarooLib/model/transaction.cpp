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

#include "transaction.h"

#include <QSet>
#include <QXmlStreamReader>
#include <set>

#include "../controller/io.h"
#include "account.h"
#include "currency.h"
#include "documentmanager.h"
#include "modelexception.h"
#include "payee.h"
#include "pricemanager.h"
#include "security.h"

namespace KLib {

namespace ClearedStatus {
QString statusToDisplay(int _status) {
  switch (_status) {
    case Cleared:
      return "C";
    case Reconciled:
      return "R";
    default:
      return "";
  }
}
}  // namespace ClearedStatus

QString Transaction::Split::formattedAmount() const {
  return Account::getTopLevel()->account(idAccount)->formatAmount(amount);
}

QString Transaction::Split::invertedFormattedAmount() const {
  return Account::getTopLevel()->account(idAccount)->formatAmount(amount * -1);
}

Transaction::Transaction()
    : m_clearedStatus(ClearedStatus::None),
      m_flagged(false),
      m_idPayee(Constants::NO_ID),
      m_isCurrencyExchange(false),
      m_properties(new Properties()) {
  connect(m_properties, SIGNAL(modified()), this, SIGNAL(modified()));
}

Transaction* Transaction::copyTo(int _idTo) const {
  Transaction* c = new Transaction();
  c->m_id = _idTo;
  copyTo(c);
  return c;
}

void Transaction::copyTo(Transaction* _other) const {
  _other->m_no = m_no;
  _other->m_clearedStatus = m_clearedStatus;
  _other->m_flagged = m_flagged;
  _other->m_date = m_date;
  _other->m_memo = m_memo;
  _other->m_idPayee = m_idPayee;
  _other->m_isCurrencyExchange = m_isCurrencyExchange;
  _other->m_splits = m_splits;
  _other->m_note = m_note;
  _other->m_properties = new Properties();

  for (const QString& s : m_properties->keys()) {
    _other->m_properties->set(s, m_properties->get(s));
  }
}

Transaction::~Transaction() { delete m_properties; }

void Transaction::setNo(const QString& _no) {
  m_no = _no;

  if (!onHoldToModify()) emit modified();
}

void Transaction::setClearedStatus(int _status) {
  if (_status < 0 || _status >= ClearedStatus::NumStatus)
    ModelException::throwException(tr("Invalid cleared status"), this);

  m_clearedStatus = _status;

  if (!onHoldToModify()) emit modified();
}

void Transaction::setFlagged(bool _flagged) {
  m_flagged = _flagged;

  if (!onHoldToModify()) emit modified();
}

void Transaction::setDate(const QDate& _date) {
  if (!_date.isValid()) {
    ModelException::throwException(tr("The date is invalid"), this);
  }

  if (m_date != _date) {
    QDate old = m_date;
    m_date = _date;
    emit transactionDateChanged(old);

    if (!onHoldToModify()) emit modified();
  }
}

void Transaction::setMemo(const QString& _memo) {
  m_memo = _memo;

  if (!onHoldToModify()) emit modified();
}

void Transaction::setNote(const QString& _note) {
  m_note = _note;

  if (!onHoldToModify()) emit modified();
}

void Transaction::setIdPayee(int _idPayee) {
  if (_idPayee != Constants::NO_ID) {
    PayeeManager::instance()->get(_idPayee);
  }

  m_idPayee = _idPayee;

  if (!onHoldToModify()) emit modified();
}

void Transaction::setAttachments(const QSet<int>& _attachments) {
  // Check if the documents are valid
  for (int docId : _attachments) {
    if (!DocumentManager::instance()->contains(docId)) {
      ModelException::throwException(
          tr("Document %1 does not exists.").arg(docId), this);
      return;
    }
  }

  // Check if some no-longer referenced documents exist
  m_attachments.subtract(_attachments);

  for (int docId : m_attachments) {
    try {
      DocumentManager::instance()->remove(docId);
    } catch (...) {
    }  // If exception, then document does not exist anyways, which is OK.
  }

  m_attachments = _attachments;

  if (!onHoldToModify()) emit modified();
}

Transaction::Split Transaction::split(int _i) const {
  if (_i < 0 || _i >= m_splits.count()) {
    ModelException::throwException(tr("Invalid index %1").arg(_i), this);
    return Split();
  }

  return m_splits[_i];
}

void Transaction::setSplits(const QList<Split>& _splits) {
  if (_splits.isEmpty()) {
    ModelException::throwException(tr("The splits are empty."), this);
  }

  // Check that the splits balance and that the accounts exist
  if (!splitsBalance(_splits)) {
    ModelException::throwException(tr("The splits do not balance."), this);
  }

  if (m_splits.isEmpty()) {
    m_splits.append(_splits);

    for (const Split& s : m_splits) {
      emit splitAdded(s);
    }
  } else {
    // Try to match the splits.
    QList<Split> oldSplits = m_splits;
    m_splits.clear();

    // To send signals AFTER everything in the transaction is fixed.
    QList<Split> amountChangedSplits;
    QList<Split> addedSplits;
    QList<Split> memoChangedSplits;

    for (Split s : _splits) {
      bool found = false;

      QMutableListIterator<Split> i(oldSplits);
      while (i.hasNext()) {
        Split old = i.next();

        if (old.idAccount == s.idAccount) {
          if (old.amount != s.amount || old.currency != s.currency) {
            amountChangedSplits.append(s);
          }

          if (old.memo != s.memo) {
            memoChangedSplits.append(s);
          }

          i.remove();
          m_splits.append(s);
          found = true;
          break;
        }
      }

      if (!found) {
        m_splits.append(s);
        addedSplits.append(s);
      }
    }

    for (const Split& s : oldSplits) {
      emit splitRemoved(s);
    }

    for (const Split& s : addedSplits) {
      emit splitAdded(s);
    }

    for (const Split& s : amountChangedSplits) {
      emit splitAmountChanged(s);
    }

    for (const Split& s : memoChangedSplits) {
      emit splitMemoChanged(s);
    }
  }

  checkIfCurrencyExchange();

  if (!onHoldToModify()) emit modified();
}

bool Transaction::splitsBalance(const QList<Transaction::Split>& _splits) {
  QHash<QString, Amount> totals;
  std::set<int> viewed_accounts;

  for (const Split& s : _splits) {
    Account* a = Account::getTopLevel()->getChild(s.idAccount);
    if (!viewed_accounts.insert(s.idAccount).second) {
      ModelException::throwException(
          tr("Accounts may not be included multiple times in a transaction."),
          nullptr);
    }

    // Transaction currency is only for currency transactions.
    if (!a->mainCurrency().isEmpty()) {
      if (s.currency.isEmpty() || !a->allCurrencies().contains(s.currency))
        return false;

      totals[s.currency] += s.amount;
    } else {
      totals[QString::number(a->idSecurity())] += s.amount;
    }
  }

  for (Amount a : totals) {
    if (a != 0) return false;
  }

  return true;
}

Amount Transaction::totalForAccount(int _idAccount,
                                    const QList<Split>& _splits) {
  Amount tot = 0;
  QString cur = Account::getTopLevel()->account(_idAccount)->mainCurrency();

  for (const Split& s : _splits) {
    if (s.idAccount == _idAccount && cur == s.currency) {
      tot += s.amount;
    } else if (s.idAccount == _idAccount)  // Different currency
    {
      tot += s.amount * PriceManager::instance()->rate(s.currency, cur);
    }
  }

  return tot;
}

Balances Transaction::totalsForAccount(int _idAccount,
                                       const QList<Split>& _splits) {
  Balances tot;

  for (const Split& s : _splits) {
    if (s.idAccount == _idAccount) {
      tot.add(s.currency, s.amount);
    }
  }

  return tot;
}

QString Transaction::splitsImbalances(
    const QList<KLib::Transaction::Split>& _splits) {
  QHash<QString, Amount> totals;

  for (const Split& s : _splits) {
    if (s.amount == 0) continue;

    if (s.idAccount == Constants::NO_ID) {
      totals[tr("Unknown")] += s.amount;
    } else {
      Account* a = Account::getTopLevel()->account(s.idAccount);

      if (a->idSecurity() == Constants::NO_ID) {
        totals[s.currency] += s.amount;
      } else {
        totals[SecurityManager::instance()->get(a->idSecurity())->symbol()] +=
            s.amount;
      }
    }
  }

  QString imbalances;

  for (auto i = totals.begin(); i != totals.end(); ++i) {
    if (i.value() != 0) {
      imbalances += tr(" <b>%1: %2</b>").arg(i.key()).arg(i.value().toString());
    } else {
      imbalances += tr(" %1: %2").arg(i.key()).arg(i.value().toString());
    }
  }

  return imbalances;
}

bool Transaction::isCurrencyExchange(
    const QList<KLib::Transaction::Split>& _splits) {
  if (_splits.count() == 4) {
    QHash<QString, Amount> sums;
    int numTrading = 0;

    for (const Split& s : _splits) {
      sums[s.currency] += s.amount;

      if (Account::getTopLevel()->account(s.idAccount)->type() ==
          AccountType::TRADING) {
        ++numTrading;
      }
    }

    // Check if everything is 0
    if (sums.count() == 2 && numTrading == 2) {
      for (Amount& a : sums) {
        if (a != 0) {
          return false;
        }
      }

      return true;
    }
  }

  return false;
}

bool Transaction::relatedTo(int _idAccount) const {
  for (const Split& s : m_splits) {
    if (s.idAccount == _idAccount) {
      return true;
    }
  }

  return false;
}

Balances Transaction::totalFor(int _idAccount) const {
  Balances bal;
  for (const Split& s : m_splits) {
    if (s.idAccount == _idAccount) {
      bal.add(s.currency, s.amount);
    }
  }

  return bal;
}

Amount Transaction::totalForInMainCurrency(int _idAccount) const {
  return totalFor(_idAccount)
      .inCurrency(Account::getTopLevel()->mainCurrency());
}

Amount Transaction::totalForInAccountCurrency(int _idAccount) const {
  return totalFor(_idAccount)
      .inCurrency(Account::getTopLevel()->account(_idAccount)->mainCurrency());
}

void Transaction::checkIfCurrencyExchange() {
  QSet<QString> currencies;
  int numTrading = 0;

  for (const Split& s : m_splits) {
    if (!s.currency.isEmpty()) {
      currencies << s.currency;

      if (Account::getTopLevel()->account(s.idAccount)->type() ==
          AccountType::TRADING) {
        ++numTrading;
      }
    }
  }

  m_isCurrencyExchange =
      currencies.count() == 2 && numTrading == 2 && m_splits.count() == 4;
}

void Transaction::addTradingSplits(QList<Split>& _splits) {
  QHash<QString, Amount> totalsC;
  QHash<int, Amount> totalsI;

  for (const Split& s : _splits) {
    Account* a = Account::getTopLevel()->getChild(s.idAccount);

    if (a->idSecurity() != Constants::NO_ID) {
      totalsI[a->idSecurity()] += s.amount;
    } else if (!s.currency.isEmpty()) {
      totalsC[s.currency] += s.amount;
    }
  }
  for (auto i = totalsC.begin(); i != totalsC.end(); ++i) {
    if (i.value() != 0)  // Missing split for this currency
    {
      Account* tr = CurrencyManager::instance()->get(i.key())->tradingAccount();

      if (!tr) {
        tr = Account::createCurrencyTradingAccount(i.key());
      }

      _splits << Split(-i.value(), tr->id(), i.key());
    }
  }
  for (auto i = totalsI.begin(); i != totalsI.end(); ++i) {
    if (i.value() != 0)  // Missing split for this investment
    {
      Account* tr = SecurityManager::instance()->get(i.key())->tradingAccount();

      if (!tr) {
        tr = Account::createSecurityTradingAccount(i.key());
      }

      _splits << Split(-i.value(), tr->id(), QString());
    }
  }
}

void Transaction::load(QXmlStreamReader& _reader) {
  QXmlStreamAttributes attributes = _reader.attributes();

  m_id = IO::getAttribute("id", attributes).toInt();
  m_no = IO::getOptAttribute("no", attributes);
  m_flagged = IO::getOptAttribute("flagged", attributes, "false") == "true";
  m_clearedStatus =
      IO::getOptAttribute("cleared", attributes, ClearedStatus::None).toInt();
  m_date = QDate::fromString(IO::getAttribute("date", attributes), Qt::ISODate);
  m_memo = IO::getOptAttribute("memo", attributes);
  m_note = IO::getOptAttribute("note", attributes);
  m_idPayee =
      IO::getOptAttribute("payee", attributes, Constants::NO_ID).toInt();

  QStringList attachments =
      IO::getOptAttribute("attachments", attributes, QString())
          .split(",", QString::SkipEmptyParts);

  for (const QString& s : attachments) {
    m_attachments.insert(s.toInt());
  }

  if (_reader.readNextStartElement()) {
    while (!(_reader.tokenType() == QXmlStreamReader::EndElement &&
             _reader.name() == StdTags::TRANSACTION)) {
      if (_reader.tokenType() == QXmlStreamReader::StartElement &&
          _reader.name() == StdTags::SPLIT) {
        Split s;
        attributes = _reader.attributes();

        s.amount =
            Amount::fromStoreable(IO::getAttribute("amount", attributes));
        s.memo = IO::getOptAttribute("memo", attributes);
        s.idAccount = IO::getAttribute("account", attributes).toInt();
        s.currency = IO::getAttribute("currency", attributes);
        s.userData = IO::getOptAttribute("userdata", attributes);

        m_splits << s;
      } else if (_reader.tokenType() == QXmlStreamReader::StartElement &&
                 _reader.name() == StdTags::PROPERTIES) {
        m_properties->load(_reader);
      }

      _reader.readNext();
    }
  }
}

void Transaction::save(QXmlStreamWriter& _writer) const {
  _writer.writeStartElement(StdTags::TRANSACTION);
  _writer.writeAttribute("id", QString::number(m_id));
  _writer.writeAttribute("no", m_no);
  _writer.writeAttribute("flagged", m_flagged ? "true" : "false");
  _writer.writeAttribute("cleared", QString::number(m_clearedStatus));
  _writer.writeAttribute("date", m_date.toString(Qt::ISODate));
  _writer.writeAttribute("memo", m_memo);
  _writer.writeAttribute("note", m_note);
  _writer.writeAttribute("payee", QString::number(m_idPayee));

  QStringList listAttachments;
  for (int idDoc : m_attachments) {
    listAttachments.append(QString::number(idDoc));
  }

  if (!listAttachments.isEmpty()) {
    _writer.writeAttribute("attachments", listAttachments.join(","));
  }

  for (Split s : m_splits) {
    _writer.writeEmptyElement(StdTags::SPLIT);
    _writer.writeAttribute("amount", s.amount.toStoreable());
    _writer.writeAttribute("memo", s.memo);
    _writer.writeAttribute("account", QString::number(s.idAccount));
    _writer.writeAttribute("currency", s.currency);
    _writer.writeAttribute("userdata", s.userData);
  }

  m_properties->save(_writer);

  _writer.writeEndElement();
}

}  // namespace KLib
