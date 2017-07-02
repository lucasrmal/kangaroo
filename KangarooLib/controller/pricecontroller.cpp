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

#include "pricecontroller.h"
#include "../model/pricemanager.h"
#include "../model/security.h"
#include  <QFont>

namespace KLib
{

    PriceController::PriceController(QObject *parent) :
        QAbstractItemModel(parent)
    {
        loadData();

        //Connections
        connect(PriceManager::instance(), SIGNAL(exchangePairAdded(KLib::ExchangePair*)),
                this, SLOT(onExchangePairAdded(KLib::ExchangePair*)));
        connect(PriceManager::instance(), SIGNAL(exchangePairRemoved(KLib::ExchangePair*)),
                this, SLOT(onExchangePairRemoved(KLib::ExchangePair*)));
        connect(PriceManager::instance(), SIGNAL(rateSet(KLib::ExchangePair*,QDate)),
                this, SLOT(onRateSet(KLib::ExchangePair*,QDate)));
        connect(PriceManager::instance(), SIGNAL(rateRemoved(KLib::ExchangePair*,QDate)),
                this, SLOT(onRateRemoved(KLib::ExchangePair*,QDate)));
    }

    PriceController::~PriceController()
    {
        delete m_root;
    }

    QVariant PriceController::data(const QModelIndex& _index, int _role) const
    {
        if (!_index.isValid())
            return QVariant();

        PriceItem *item = static_cast<PriceItem*>(_index.internalPointer());

        if (_role == Qt::DisplayRole)
        {
            switch (item->itemType)
            {
            case PriceItem::Root:
                return QVariant();

            case PriceItem::PriceTypes:
                switch (_index.column())
                {
                case PriceColumn::PAIR:
                    return item->priceType == PriceType::Currency ? tr("Currency")
                                                                  : tr("Security");
                default:
                    return QVariant();
                }
            case PriceItem::Exchange:
                switch (_index.column())
                {
                case PriceColumn::PAIR:
                    return tr("%1 to %2").arg(item->pair->isSecurity() ? item->pair->securityFrom()->symbol()
                                                                       : item->pair->from())
                                              .arg(item->pair->to());
                default:
                    return QVariant();
                }
            case PriceItem::Rate:
                switch (_index.column())
                {
                case PriceColumn::PAIR:
                    return QString::number(item->value);

                case PriceColumn::DATE:
                    return item->date.toString(Qt::ISODate);

                default:
                    return QVariant();
                }
            default:
                return QVariant();
            }
        }
        else if (_role == Qt::EditRole && item->itemType == PriceItem::Rate && _index.column() == PriceColumn::PAIR)
        {
            return QString::number(item->value);
        }
        else if (_role == Qt::FontRole && (item->itemType == PriceItem::Exchange || item->itemType == PriceItem::PriceTypes))
        {
            QFont f;
            f.setBold(true);
            return f;
        }

        return QVariant();
    }

    Qt::ItemFlags PriceController::flags(const QModelIndex& _index) const
    {
        if (!_index.isValid())
            return 0;

        PriceItem* item = static_cast<PriceItem*>(_index.internalPointer());

        if (item && item->itemType == PriceItem::Rate && _index.column() == PriceColumn::PAIR)
        {
            return QAbstractItemModel::flags(_index) | Qt::ItemIsEditable;
        }

        return QAbstractItemModel::flags(_index);
    }

    QVariant PriceController::headerData(int _section, Qt::Orientation _orientation, int _role) const
    {
        if (_role != Qt::DisplayRole || _orientation != Qt::Horizontal)
            return QVariant();

        switch (_section)
        {
        case PriceColumn::PAIR:
            return tr("Price");

        case PriceColumn::DATE:
            return tr("Date");

        default:
            return QVariant();
        }
    }

    QModelIndex PriceController::index(int _row, int _column, const QModelIndex& _parent) const
    {
        if (!hasIndex(_row, _column, _parent))
            return QModelIndex();

        PriceItem* parentItem;

        if (!_parent.isValid())
        {
            parentItem = m_root;
        }
        else
        {
            parentItem = static_cast<PriceItem*>(_parent.internalPointer());
        }

        try
        {
            return createIndex(_row, _column, parentItem->children[_row]);
        }
        catch (std::exception&)
        {
            return QModelIndex();
        }
    }

    QModelIndex PriceController::parent(const QModelIndex& _index) const
    {
        if (!_index.isValid())
            return QModelIndex();

        PriceItem* child = static_cast<PriceItem*>(_index.internalPointer());


        PriceItem *parent = child->parent;

        if (!parent)
            return QModelIndex();

        return createIndex(parent->index, 0, parent);
    }

    int PriceController::rowCount(const QModelIndex& _parent) const
    {
        if (_parent.column() > 0)
            return 0;

        if (!_parent.isValid()) // Act as top level
            return PriceType::NumTypes;

        PriceItem* i = static_cast<PriceItem*>(_parent.internalPointer());

        if (i)
        {
            return i->children.count();
        }

        return 0;
    }

    int PriceController::columnCount(const QModelIndex& _parent) const
    {
        Q_UNUSED(_parent)

        return PriceColumn::NumColumns;
    }

    bool PriceController::setData(const QModelIndex& _index, const QVariant& _value, int _role)
    {
        if (!_index.isValid() || _role != Qt::EditRole)
            return false;

        PriceItem* i = static_cast<PriceItem*>(_index.internalPointer());

        if (!i || i->itemType != PriceItem::Rate || _index.column() != PriceColumn::PAIR)
            return false;

        i->parent->pair->set(i->date, _value.toDouble());
        i->value = _value.toDouble();
        emit dataChanged(_index, _index);
        return true;
    }

    void PriceController::loadData()
    {
        m_root = new PriceItem(PriceItem::Root);

        PriceItem* sec = new PriceItem(PriceItem::PriceTypes);
        PriceItem* cur = new PriceItem(PriceItem::PriceTypes);

        sec->priceType = PriceType::Security;
        cur->priceType = PriceType::Currency;

        sec->addToParent(m_root);
        cur->addToParent(m_root);

        for (ExchangePair* p : PriceManager::instance()->pairs())
        {
            PriceItem* pi = new PriceItem(PriceItem::Exchange);
            pi->pair = p;

            if (p->isSecurity())
            {
                pi->addToParent(sec);
            }
            else
            {
                pi->addToParent(cur);
            }

            for (ExchangePair::Rate r : p->rates())
            {
                PriceItem* ri = new PriceItem(PriceItem::Rate);
                ri->date = r.first;
                ri->value = r.second;

                ri->addToParent(pi);
            }
        }
    }

    void PriceController::onExchangePairAdded(ExchangePair* _p)
    {
        // Check if it's there already

        PriceItem* parent = _p->isSecurity() ? m_root->children[0]
                                             : m_root->children[1];

        for (PriceItem* i : parent->children)
        {
            if (i->pair == _p)
                return;
        }

        beginInsertRows(createIndex(parent->index, 0, parent), parent->children.count(), parent->children.count());
        PriceItem* i = new PriceItem(PriceItem::Exchange);
        i->addToParent(parent);
        i->pair = _p;
        endInsertRows();
    }

    void PriceController::onExchangePairRemoved(ExchangePair* _p)
    {
        //Find it
        PriceItem* parent = NULL, *toDelete = NULL;

        for (int i = 0; i < m_root->children.count() && !parent; ++i)
        {
            for (PriceItem* p : m_root->children[i]->children)
            {
                if (p->pair == _p)
                {
                    parent = m_root->children[i];
                    toDelete = p;
                    break;
                }
            }
        }

        if (!parent)
            return;

        beginRemoveRows(createIndex(parent->index, 0, parent), toDelete->index, toDelete->index);
        parent->children.removeAt(toDelete->index);

        for (int i = toDelete->index; i < parent->children.count(); ++i)
        {
            parent->children[i]->index = i;
        }

        endRemoveRows();
        delete toDelete;
    }

    void PriceController::onRateSet(ExchangePair* _p, const QDate& _date)
    {
        PriceItem* parent = NULL;

        for (int i = 0; i < m_root->children.count() && !parent; ++i)
        {
            for (PriceItem* p : m_root->children[i]->children)
            {
                if (p->pair == _p)
                {
                    parent = p;
                    break;
                }
            }
        }

        if (!parent)
            return;

        // Find the index of the modified rate
        bool found = false;
        int row = 0;

        for (row = 0; row < parent->children.count() && _date >= parent->children[row]->date; ++row)
        {
            if (parent->children[row]->date == _date)
            {
                found = true;
                parent->children[row]->value = _p->on(_date);
                emit dataChanged(createIndex(row, PriceColumn::PAIR, parent->children[row]),
                                 createIndex(row, PriceColumn::PAIR, parent->children[row]));
            }
        }

        if (!found) // Need to add it at row
        {
            beginInsertRows(createIndex(parent->index, 0, parent), row, row);

            PriceItem* i = new PriceItem(PriceItem::Rate);
            i->date = _date;
            i->value = _p->on(_date);
            i->index = row;
            i->parent = parent;
            parent->children.insert(row, i);
            endInsertRows();
        }
    }

    void PriceController::onRateRemoved(ExchangePair* _p, const QDate& _date)
    {
        //Find the parent (exchange pair)
        PriceItem* parent = NULL;

        for (int i = 0; i < m_root->children.count() && !parent; ++i)
        {
            for (PriceItem* p : m_root->children[i]->children)
            {
                if (p->pair == _p)
                {
                    parent = p;
                    break;
                }
            }
        }

        if (!parent)
            return;

        // Find the index of the deleted rate and remove it
        for (int row = 0; row < parent->children.count() && _date >= parent->children[row]->date; ++row)
        {
            if (parent->children[row]->date == _date)
            {
                beginRemoveRows(createIndex(parent->index, 0, parent), row, row);
                parent->children.removeAt(row);
                endRemoveRows();
            }
        }
    }

    PriceItem::PriceItem(PriceItem::Type _itemType) :
        itemType(_itemType),
        pair(NULL),
        value(0),
        parent(NULL),
        index(0)
    {
    }

    PriceItem::~PriceItem()
    {
        for (PriceItem* c : children)
        {
            delete c;
        }
    }

    void PriceItem::addToParent(PriceItem* _parent)
    {
        parent = _parent;
        index = parent->children.count();
        parent->children.append(this);
    }






}
