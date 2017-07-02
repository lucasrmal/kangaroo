#ifndef ERRORS_H
#define ERRORS_H

#include <QList>
#include <QPair>

namespace KLib
{
    enum ErrorType
    {
        Warning,
        Critical
    };

    typedef QPair<ErrorType, QString> Error;
    typedef QList<Error> ErrorList;
}

#endif // ERRORS_H

