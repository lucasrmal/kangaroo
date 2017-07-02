/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2008-2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
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

/**
  @file  isaveposition.h
  This file contains the method declarations for the ISavePosition interface

  @author   Lucas Rioux Maldague
  @date     June 12th 2010
  @version  3.0

*/

#ifndef ISAVEPOSITION_H
#define ISAVEPOSITION_H

#include <QString>
#include <QSize>
#include <QPoint>

class QWidget;

namespace KLib {

    /**
      The ISavePosition allows a window to save its position and size when the application is closed. To implement
      this functionnality, you should inherit it (using multiple inheritance). Then, in your window constructor, call the loadPosition() method.
      Also, don't forget to re-implement the window's close event and call savePosition().

      You also need to provide a unique name (it's recommended to pass the class name, ex: MainWindow). This unique name is used to save the position and size of the window in
      the settings. If many windows share the same unique name, their position and size will overwrite each other when they
      are closed.

      @brief Saves a window's position and size
    */
    class ISavePosition
    {
        public:
            virtual ~ISavePosition() {}

        protected:
            /**
              Class constructor

              @param[in] p_uniqueName The window's unique name
              @param[in] p_widget A pointer to the widget
              @param[in] p_defaultSize The default window size. This will be the window's size if it's opened for the first time.
              @param[in] p_defaultPosition The default window position. This will be the window's position if it's opened for the first time.
            */
            ISavePosition(const QString& p_uniqueName, QWidget* p_widget, const QSize& p_defaultSize = QSize(480, 400), const QPoint& p_defaultPosition = QPoint(200, 200)) :
                    m_uniqueName(p_uniqueName),
                    m_widget(p_widget),
                    m_defaultSize(p_defaultSize),
                    m_defaultPosition(p_defaultPosition) {}

            /**
              Saves the window position and size. It must be called from the window's close event.
            */
            void savePosition();

            /**
              Loads the window position and size. It must be called from the window's constructor.
            */
            void loadPosition();

        private:
            QString  m_uniqueName;
            QWidget* m_widget;
            QSize    m_defaultSize;
            QPoint   m_defaultPosition;
    };


} //Namespace

#endif // ISAVEPOSITION_H
