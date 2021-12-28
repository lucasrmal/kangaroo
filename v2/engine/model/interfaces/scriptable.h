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

#ifndef SCRIPTABLE_H
#define SCRIPTABLE_H

#include <QScriptEngine>
#include <QString>
#include <QtDebug>
#include <QScriptValueIterator>

#define K_SCRIPTABLE(Name) public: static QString className() { return QString(#Name); } private:

namespace KLib
{

    class Scriptable
    {
        virtual void addToEngine(QScriptEngine* _engine) const = 0;

        friend class ScriptEngine;
    };


    template<class T>
    class ScriptableObject : public Scriptable
    {
        static QScriptValue toScriptValue(QScriptEngine *engine, T* const &in)
        {
            return engine->newQObject(in);
        }

        static void fromScriptValue(const QScriptValue &object, T* &out)
        {
            out = qobject_cast<T*>(object.toQObject());
        }

        static QScriptValue objectListToScriptValue(QScriptEngine* eng, const QList<T*>& _list)
        {
          QScriptValue a = eng->newObject();

          int i = 0;
          for (T* o: _list)
          {
              a.setProperty(i, toScriptValue(eng, o));
              ++i;
          }

          a.setProperty("length", i);

          return a;
        }

        static void objectListFromScriptValue( const QScriptValue& value, QList<T*>& _list)
        {
          QScriptValueIterator i(value);
          _list.clear();

          while (i.hasNext())
          {
              i.next();
              _list.append(qobject_cast<T*>(i.value().toQObject()));
          }
        }

        static QScriptValue objectLinkedListToScriptValue(QScriptEngine* eng, const QLinkedList<T*>& _list)
        {
          QScriptValue a = eng->newObject();

          int i = 0;
          for (T* o: _list)
          {
              a.setProperty(i, toScriptValue(eng, o));
              ++i;
          }

          a.setProperty("length", i);

          return a;
        }

        static void objectLinkedListFromScriptValue( const QScriptValue& value, QLinkedList<T*>& _list)
        {
          QScriptValueIterator i(value);
          _list.clear();

          while (i.hasNext())
          {
              i.next();
              _list.append(qobject_cast<T*>(i.value().toQObject()));
          }
        }

        void addToEngine(QScriptEngine* _engine) const
        {
            qScriptRegisterMetaType(_engine, toScriptValue, fromScriptValue);
            qScriptRegisterMetaType(_engine, objectListToScriptValue, objectListFromScriptValue);
            qScriptRegisterMetaType(_engine, objectLinkedListToScriptValue, objectLinkedListFromScriptValue);
        }

    };

    template<class T>
    class ScriptableInstance : public Scriptable
    {
        static QScriptValue objInstance(QScriptContext*, QScriptEngine* engine)
        {
            QObject *object = T::instance();
            return engine->newQObject(object, QScriptEngine::QtOwnership);
        }

        void addToEngine(QScriptEngine* _engine) const
        {
            QScriptValue constructor = _engine->newFunction(objInstance);
            QScriptValue metaObject = _engine->newQMetaObject(&T::staticMetaObject, constructor);

            _engine->globalObject().setProperty(T::className(), metaObject);
        }

    };

    template<class T, T* (*INSTANCE)()>
    class ScriptableInstance2 : public Scriptable
    {
        static QScriptValue objInstance(QScriptContext*, QScriptEngine* engine)
        {
            QObject *object = static_cast<QObject*>(INSTANCE());
            return engine->newQObject(object, QScriptEngine::QtOwnership);
        }

        void addToEngine(QScriptEngine* _engine) const
        {
            QScriptValue constructor = _engine->newFunction(objInstance);
            QScriptValue metaObject = _engine->newQMetaObject(&T::staticMetaObject, constructor);

            _engine->globalObject().setProperty(T::className(), metaObject);
        }

    };

    template<class T,
             QScriptValue (*TO)(QScriptEngine*, const T&s),
             void (*FROM)(const QScriptValue&, T &s)>
    class ScriptableStructure : public Scriptable
    {
        static QScriptValue createMyStruct(QScriptContext*, QScriptEngine* engine)
        {
            return engine->toScriptValue(T());
        }

        void addToEngine(QScriptEngine* _engine) const
        {
            QScriptValue ctor = _engine->newFunction(createMyStruct);
            _engine->globalObject().setProperty(T::className(), ctor);

            qScriptRegisterMetaType(_engine, TO, FROM);
        }
    };

}

#endif // SCRIPTABLE_H
