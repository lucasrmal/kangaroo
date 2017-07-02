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
  This file contains the method definitions for the CentralWidget class

  @author   Lucas Rioux Maldague
  @date     June 12th
  @version  3.0

*/

#include "tabinterface.h"
#include <KangarooLib/ui/core.h>

#include <QListWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QPushButton>

TabInterface* TabInterface::m_instance = nullptr;
const QTabBar::ButtonPosition TabInterface::CLOSE_BUTTON_POSITION = QTabBar::RightSide;

TabInterface::TabInterface(QWidget *parent) :
    QTabWidget(parent)
{
    setTabsClosable(false);
    setMovable(true);
}

TabInterface* TabInterface::instance()
{
    if ( !m_instance)
        m_instance = new TabInterface();
    return m_instance;
}

void TabInterface::deleteInstance()
{
//    m_instance->deleteLater();
//    m_instance = NULL;

}

void TabInterface::addRegisteredTab(QWidget* _widget, const QString& _title, const QString& _identifier, bool _closeable, const QIcon& _icon)
{
    if (!m_tabs.contains(_identifier))
    {
        QString newTitle = _title;
        newTitle.replace("&", "&&");
        addTab(_widget, _icon, newTitle);

        m_tabs.insert(_identifier, _widget);

        if (_closeable)
        {
            m_closeableTabs.append(_widget);
            QPushButton *closeButton = new QPushButton(this);
            closeButton->setIcon(KLib::Core::icon("close"));
            closeButton->setMaximumSize(16,16);
            closeButton->setFlat(true);
            closeButton->setObjectName(_identifier);

            connect(closeButton, SIGNAL(clicked()), this, SLOT(closeTab()));

            // next line sets closeButton in right corner on tab with index 0
            tabBar()->setTabButton(indexOf(_widget), CLOSE_BUTTON_POSITION, closeButton);
        }

        setCurrentWidget(_widget);
    }
    else
    {
        setFocus(_identifier);
    }
}

void TabInterface::setFocus(const QString& _identifier)
{
    if (m_tabs.contains(_identifier))
    {
        setCurrentIndex(indexOf(m_tabs[_identifier]));
    }
}

void TabInterface::closeTab()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    closeTab(btn->objectName());
}

void TabInterface::closeAllTabs()
{
    while (count())
    {
        removeTab(0);
    }

    m_closeableTabs.clear();
    m_tabs.clear();
    emit allTabsClosed();
}

void TabInterface::closeTab(const QString& _identifier)
{
    QWidget* w = m_tabs[_identifier];
    if (m_closeableTabs.contains(w))
    {
        m_closeableTabs.removeOne(w);
        m_tabs.remove(_identifier);
        removeTab(indexOf(w));
        emit tabClosed(w);
    }
}

void TabInterface::closeAllCloseableTabs()
{
    QSet<QWidget*> removed;

    for (QWidget* w : m_closeableTabs)
    {
        removeTab(indexOf(w));
        emit tabClosed(w);
        removed << w;
    }

    m_closeableTabs.clear();

    QMutableHashIterator<QString, QWidget*> i(m_tabs);

    while (i.hasNext())
    {
        i.next();

        if (removed.contains(i.value()))
        {
            i.remove();
        }
    }
}

void TabInterface::closeCurrentTab()
{
    QWidget* button = tabBar()->tabButton(currentIndex(), CLOSE_BUTTON_POSITION);

    if (button)
    {
        static_cast<QPushButton*>(button)->click();
    }
}
