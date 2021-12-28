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

#ifndef MODELEXCEPTION_H
#define MODELEXCEPTION_H

#include <QString>
#include <exception>

class QScriptEngine;

namespace KLib
{

class IStored;

class ModelException : public std::exception
{
        ModelException(const QString& _what, const IStored* _sender) throw();
public:

    virtual ~ModelException()  throw() {}

    QString description() const throw() { return m_description; }
    const IStored* sender() const throw() { return m_sender; }

    virtual const char* what() const throw()
    {
        return m_description.toLocal8Bit().data();
    }

    static void throwException(const QString& _what, const IStored* _sender);
    static void beginScriptMode(QScriptEngine* _engine);
    static void endScriptMode();

private:
    QString m_description;
    const IStored* m_sender;

    static QScriptEngine* m_engine;
};

}

#endif // MODELEXCEPTION_H
