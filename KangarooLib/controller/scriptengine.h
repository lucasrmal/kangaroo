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

#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QString>
#include <QScriptEngine>
#include <QLocale>


namespace KLib
{
    class Scriptable;

    class ScriptEngine
    {
        public:

            bool execute(const QString& _script, QString& _output) const;

            void registerScriptable(Scriptable* _s);

            static ScriptEngine* instance();

        private:
            ScriptEngine();

            QList<Scriptable*> m_instances;

            static ScriptEngine* m_instance;
    };

    class Locale : public QObject
    {
        Q_OBJECT

        public:
            Locale(QObject* _parent = nullptr) : QObject(_parent) {}

            Q_INVOKABLE QString toNum(double _num) const;

        private:
            QLocale m_locale;
    };

}

#endif // SCRIPTENGINE_H
