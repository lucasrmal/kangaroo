#ifndef POSITIONSMODEL_H
#define POSITIONSMODEL_H

#include <QAbstractTableModel>
#include <KangarooLib/amount.h>

namespace KLib
{
    class Account;
    class Security;
    class Currency;
}



class PositionsModel : public QAbstractTableModel
{
    public:
        //PositionsModel(KLib::Account* _parent, QObject *parent = nullptr);

};

#endif // POSITIONSMODEL_H
