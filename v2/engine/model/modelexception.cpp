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

#include "modelexception.h"
#include <QScriptEngine>

namespace KLib {

    QScriptEngine* ModelException::m_engine = nullptr;

    ModelException::ModelException(const QString &_what, const IStored *_sender) throw() :
        m_description(_what),
        m_sender(_sender)
    {
    }

    void ModelException::throwException(const QString& _what, const IStored* _sender)
    {
        if (!m_engine)
        {
            throw ModelException(_what, _sender);
        }
        else
        {
            m_engine->currentContext()->throwError(_what);
        }
    }

    void ModelException::beginScriptMode(QScriptEngine* _engine)
    {
        m_engine = _engine;
    }

    void ModelException::endScriptMode()
    {
        m_engine = nullptr;
    }

}
