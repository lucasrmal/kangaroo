#ifndef BUDGET_H
#define BUDGET_H

#include <QString>
#include <QList>
#include <QVector>
#include "stored.h"
#include "../interfaces/scriptable.h"

namespace KLib {

class Budget : public IStored
{
  Q_OBJECT
  K_SCRIPTABLE(Budget)

  public:
    Budget();
    ~Budget();
};

class BudgetManager : public IStored
{
  Q_OBJECT
  K_SCRIPTABLE(BudgetManager)

  public:

};

}  // namespace KLib

#endif // BUDGET_H
