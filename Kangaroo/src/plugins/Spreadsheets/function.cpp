#include "function.h"

const char Function::FUNC_BEGIN_CHAR = '=';

Function::Function() :
    m_isValid(false),
    m_lastCode(FUNC_BEGIN_CHAR)
{
}

bool Function::isFunction(const QString& _str)
{
    return _str.startsWith(FUNC_BEGIN_CHAR);
}

Function Function::evaluate(const QString& _str)
{
    return Function();
}

QString Function::toString() const
{
    if (!m_isValid)
    {
        return m_lastCode;
    }
    else
    {
        return "";
    }
}

QVariant Function::value() const
{
    if (!m_isValid)
    {
        return QObject::tr("##INVALID##");
    }
    else
    {
        return QVariant();
    }
}
