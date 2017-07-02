#include "formsetsettings.h"
#include <KangarooLib/ui/core.h>

#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <climits>
#include <QMessageBox>
#include <KangarooLib/ui/widgets/accountselector.h>

using namespace KLib;

FormSetSettings::FormSetSettings(ReportSettings& _settings, QWidget *parent) :
    CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
    m_settings(_settings)
{
    setBothTitles(tr("Report Settings"));
    setPicture(Core::pixmap("report"));

    QFormLayout* l = new QFormLayout(centralWidget());

    for (ReportSetting& s : m_settings)
    {
        QWidget* w;
        switch (s.type)
        {
        case SettingType::String:
            w = new QLineEdit(s.value.toString(), this);
            break;

        case SettingType::Date:
        {
            QDateEdit* date = new QDateEdit(s.value.toDate(), this);
            date->setCalendarPopup(true);
            w = date;
            break;
        }

        case SettingType::Year:
        {
            QSpinBox* spin = new QSpinBox(this);
            spin->setMinimum(-9999);
            spin->setMaximum(9999);
            spin->setValue(s.value.toInt());
            w = spin;
            break;
        }

        case SettingType::Month:
        {
            QComboBox* cbo = new QComboBox(this);

            for (int i = 1; i <= 12; ++i)
            {
                cbo->addItem(QDate::longMonthName(i));
            }

            cbo->setCurrentIndex(s.value.toInt()-1);
            w = cbo;
            break;
        }

        case SettingType::Boolean:
        {
            QCheckBox* chk = new QCheckBox(this);
            chk->setChecked(s.value.toBool());
            w = chk;
            break;
        }

        case SettingType::Integer:
        {
            QSpinBox* spin = new QSpinBox(this);
            spin->setValue(s.value.toInt());
            spin->setMinimum(INT_MIN);
            spin->setMaximum(INT_MAX);
            w = spin;
            break;
        }

        case SettingType::Double:
        {
            QDoubleSpinBox* spin = new QDoubleSpinBox(this);
            spin->setValue(s.value.toDouble());
            spin->setMinimum(-qInf());
            spin->setMaximum(qInf());
            w = spin;
            break;
        }

        case SettingType::List:
        {
            QComboBox* cbo = new QComboBox(this);

            for (QString str : s.list)
            {
                cbo->addItem(str);
            }

            if (cbo->findText(s.value.toString()) != -1)
            {
                cbo->setCurrentIndex(cbo->findText(s.value.toString()));
            }
            w = cbo;
            break;
        }

        case SettingType::Account:
        {
            AccountSelector* sel = new AccountSelector(s.allowPlaceholders ? Flag_PlaceholdersClosed
                                                                           : Flag_Closed,
                                                       s.flagsToAccountFlags(),
                                                       Constants::NO_ID,
                                                       this);

            if (!s.defaultVal.isEmpty())
            {
                sel->setCurrentAccount(s.defaultVal.toInt());
            }

            w = sel;
            break;
        }

        }

        l->addRow(s.description, w);
        m_widgets << w;
    }

}

void FormSetSettings::accept()
{
    for (int i = 0; i < m_settings.count(); ++i)
    {
        ReportSetting& s = m_settings[i];
        QWidget* w = m_widgets[i];

        switch (s.type)
        {
        case SettingType::String:
        {
            QLineEdit* txt = static_cast<QLineEdit*>(w);
            s.value = txt->text();
            break;
        }
        case SettingType::Date:
        {
            QDateEdit* date = static_cast<QDateEdit*>(w);
            s.value = date->date();
            break;
        }
        case SettingType::Month:
        {
            QComboBox* cbo = static_cast<QComboBox*>(w);
            s.value = cbo->currentIndex()+1;
            break;
        }
        case SettingType::Year:
        case SettingType::Integer:
        {
            QSpinBox* spin = static_cast<QSpinBox*>(w);
            s.value = spin->value();
            break;
        }
        case SettingType::Boolean:
        {
            QCheckBox* chk = static_cast<QCheckBox*>(w);
            s.value = chk->isChecked();
            break;
        }
        case SettingType::Double:
        {
            QDoubleSpinBox* spin = static_cast<QDoubleSpinBox*>(w);
            s.value = spin->value();
            break;
        }
        case SettingType::List:
        {
            QComboBox* cbo = static_cast<QComboBox*>(w);
            s.value = cbo->currentText();
            break;
        }
        case SettingType::Account:
        {
            AccountSelector* sel = static_cast<AccountSelector*>(w);

            if (!sel->currentAccount())
            {
                QMessageBox::information(this,
                                         s.description,
                                         tr("Please select an account."));
                return;
            }

            s.value = sel->currentAccount()->id();
            break;
        }
        }
    }

    done(QDialog::Accepted);
}
