/*
This file is part of CAMSEG SCM.
Copyright (C) 2008-2010 CAMSEG Technologies

CAMSEG SCM is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CAMSEG SCM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with CAMSEG SCM. If not, see <http://www.gnu.org/licenses/>.
*/
/**
  @file  centralwidget.h
  This file contains the method declarations for the CentralWidget class

  @author   Lucas Rioux Maldague
  @date     June 12th
  @version  3.0

*/

#ifndef TABINTERFACE_H
#define TABINTERFACE_H

#include <QTabWidget>
#include <QTabBar>

/**
  @brief Tab manager
*/
class TabInterface : public QTabWidget
{
    Q_OBJECT

        explicit TabInterface(QWidget *parent = nullptr);

    public:
        static TabInterface* instance();

        /**
         * @brief addTab Adds a tab with identifier _identifier if it does not already exists.
         * @param _title
         * @param _identifier
         * @param _closeable
         * @param _icon
         */
        void addRegisteredTab(QWidget* _widget, const QString& _title, const QString& _identifier, bool _closeable, const QIcon& _icon = QIcon());

        QWidget *tab(const QString& _identifier) const { return m_tabs.value(_identifier, nullptr); }

        bool containsTab(const QString& _identifier) const { return m_tabs.contains(_identifier); }

        void setFocus(const QString& _identifier);

        void closeTab(const QString& _identifier);

        bool hasTab(const QString& _identifier) { return m_tabs.contains(_identifier); }

        static const QTabBar::ButtonPosition CLOSE_BUTTON_POSITION;

    public slots:
        void closeAllCloseableTabs();
        void closeCurrentTab();


    signals:
        void tabClosed(QWidget* _tab);
        void allTabsClosed();

    private slots:
        void closeTab();

    private:
        void closeAllTabs();

        static TabInterface* m_instance;

        QHash<QString, QWidget*> m_tabs;
        QList<QWidget*> m_closeableTabs;

        static void deleteInstance();

        friend class CentralWidget;

};

#endif // TABINTERFACE_H
