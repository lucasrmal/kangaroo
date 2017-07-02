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

#ifndef DOCKWIDGETMANAGER_H
#define DOCKWIDGETMANAGER_H

#include <QObject>
#include <QHash>

class QDockWidget;
class QAction;
class QWidget;

namespace KLib
{

    class DockWidgetFactorySuper
    {
        public:

            virtual QDockWidget* createDock(QWidget* _parent) const = 0;

            virtual ~DockWidgetFactorySuper() {}

            QDockWidget* currentDock() const { return m_current; }

        protected:
            DockWidgetFactorySuper() : m_current(NULL) {}

        private:

            bool m_requiresFile;
            QAction* m_action;
            QString m_id;
            Qt::DockWidgetArea m_area;
            QDockWidget* m_current;

            friend class DockWidgetManager;
    };

    template<class T>
    class DockWidgetFactory : public DockWidgetFactorySuper
    {
        public:
            DockWidgetFactory() {}

            virtual QDockWidget* createDock(QWidget* _parent) const { return new T(_parent); }
    };

class DockWidgetManager : public QObject
{
    Q_OBJECT

        DockWidgetManager() {}

    public:
        ~DockWidgetManager();

        static DockWidgetManager*	    instance();

        void                registerDockWidget(const QString& _dockId,
                                               const QString& _actionText,
                                               const QIcon& _actionIcon,
                                               Qt::DockWidgetArea _area,
                                               bool _requiresFile,
                                               DockWidgetFactorySuper* _factory);

    private slots:
        void openDock();

    private:

        void onLoad();
        void onUnload();

        QHash<QString, DockWidgetFactorySuper*> m_docks;
        QHash<QAction*, DockWidgetFactorySuper*> m_docksByAction;

        static DockWidgetManager* m_instance;

        friend class Core;

};

}

#endif // DOCKWIDGETMANAGER_H
