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

#include "generalsettings.h"
#include "../../widgets/filechooserbox.h"
#include "../../core.h"

#include <QFormLayout>
#include <QDir>
#include <QFileInfo>
#include <QComboBox>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>
#include <QLocale>
#include <QSet>
#include <QDate>

namespace KLib {
	
    //Widget
    GeneralSettings::GeneralSettings(QWidget* parent) :  ISettingsPage(parent)
    {
        m_cboLanguage = new QComboBox(this);
        m_cboDateFormat= new QComboBox(this);
        m_spinMaxRecent = new QSpinBox(this);


        m_spinMaxRecent->setMinimum(1);
        m_spinMaxRecent->setMaximum(10);

        QFormLayout* layout = new QFormLayout(this);
        layout->addRow(tr("Language:"), m_cboLanguage);
        layout->addRow(tr("Date Format:"), m_cboDateFormat);
        layout->addRow(tr("Maximum number of recent files:"), m_spinMaxRecent);
    }

    QStringList GeneralSettings::validate()
    {
        QStringList errors;

        if (m_cboLanguage->currentIndex() < 0)
        {
            errors << tr("Please select a valid language.");
        }

        if (m_cboDateFormat->currentIndex() < 0)
        {
            errors << tr("Please select a valid date format.");
        }

        return errors;
    }

    void GeneralSettings::save()
    {
        SettingsManager* s = SettingsManager::instance();

        s->setValue("General/Language", m_cboLanguage->currentData());
        s->setValue("General/DateFormat", m_cboDateFormat->currentData());
        s->setValue("RecentFiles/MaxCount", m_spinMaxRecent->value());
    }

    void GeneralSettings::load()
    {
        loadLanguages();
        loadDateFormats();

        SettingsManager* s = SettingsManager::instance();

        int idx = m_cboLanguage->findData(s->value("General/Language"));

        if (idx == -1)
        {
            idx = m_cboLanguage->findData("en_US");
        }

        m_cboLanguage->setCurrentIndex(idx);

        m_cboDateFormat->setCurrentIndex(m_cboDateFormat->findData(s->value("General/DateFormat")));
        m_spinMaxRecent->setValue(s->value("RecentFiles/MaxCount").toInt());
    }

    void GeneralSettings::loadDateFormats()
    {
        QStringList formats;

        formats << "yyyy/MM/dd";
        formats << "yy/MM/dd";
        formats << "MM/dd/yyyy";
        formats << "MM/dd/yy";
        formats << "MMM d yyyy";

        for (const QString& s : formats)
        {
            m_cboDateFormat->addItem(QString("%1 (ex: %2)").arg(s).arg(QDate::currentDate().toString(s)), s);
        }

    }

    void GeneralSettings::loadLanguages()
    {
        typedef QPair<QLocale::Language, QLocale::Country> CL;

        QSet<CL> languages;

        languages.insert(CL(QLocale::English, QLocale::UnitedStates));


        //Find the installed translations
        QDir trDir(Core::path(Path_Translations));

        //Language files are "LANGUAGE_CODE.qm". For example: "fr.qm"

        foreach(QFileInfo object, trDir.entryInfoList(QDir::Files))
        {
            if (object.completeSuffix() == "qm")
            {
                QLocale l(object.baseName());

                if (l.language() != QLocale::C)
                {
                    languages.insert(CL(l.language(), l.country()));
                }
            }
        }

        //Now, sort them
        QList<CL> sorted = languages.toList();

        std::sort(sorted.begin(), sorted.end(), [](const CL& a, const CL& b) {

            if (QLocale::languageToString(a.first) == QLocale::languageToString(b.first))
            {
                return QLocale::countryToString(a.second) < QLocale::countryToString(b.second);
            }
            else
            {
                return QLocale::languageToString(a.first) < QLocale::languageToString(b.first);
            }
        });

        //Add them to the list
        for (CL& c : sorted)
        {
            m_cboLanguage->addItem(QString("%1/%2")
                                        .arg(QLocale::languageToString(c.first))
                                        .arg(QLocale::countryToString(c.second)),
                                   QLocale(c.first, c.second).name());
        }
    }

} //Namespace
