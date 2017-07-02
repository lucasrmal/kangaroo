#include "formmerge.h"

#include <QRadioButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/model/payee.h>
#include <KangarooLib/model/institution.h>
#include <KangarooLib/model/modelexception.h>

using namespace KLib;

FormMerge::FormMerge(const QSet<int>& _mergeSet, ManageType _type, QWidget *parent) :
    CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
    m_mergeList(_mergeSet.toList()),
    m_mergeSet(_mergeSet),
    m_type(_type)
{
    if (m_mergeList.count() < 2)
    {
        return;
    }

    QString msg, type;

    if (m_type == ManageType::Payees)
    {
        msg = tr("Select the destination payee:");
        type = tr("Payees");
    }
    else //Institutions
    {
        msg = tr("Select the destination institution:");
        type = tr("Institutions");
    }

    setPicture(Core::pixmap("merge"));
    setBothTitles(tr("Merge %1").arg(type));

    QLabel* lblExplain = new QLabel(msg);

    QVBoxLayout* layout = new QVBoxLayout(centralWidget());
    layout->addWidget(lblExplain);

    for (int id: _mergeSet)
    {
        QRadioButton* button = new QRadioButton(this);

        if (m_type == ManageType::Payees)
        {
            button->setText(PayeeManager::instance()->get(id)->name());
        }
        else
        {
            button->setText(InstitutionManager::instance()->get(id)->name());
        }

        m_options << button;
        layout->addWidget(button);
    }

    m_options[0]->setChecked(true);
}

void FormMerge::accept()
{
    int checked = -1;

    for (int i = 0; i < m_options.count(); ++i)
    {
        if (m_options[i]->isChecked())
        {
            checked = m_mergeList[i];
            break;
        }
    }

    try
    {
        if (m_type == ManageType::Payees)
        {
            if (checked == -1)
            {
                QMessageBox::information(this, tr("Merge"), tr("Select one payee."));
                return;
            }

            PayeeManager::instance()->merge(m_mergeSet, checked);
        }
        else
        {
            if (checked == -1)
            {
                QMessageBox::information(this, tr("Merge"), tr("Select one institution."));
                return;
            }

            InstitutionManager::instance()->merge(m_mergeSet, checked);
        }

        done(QDialog::Accepted);
    }
    catch (ModelException e)
    {
        QMessageBox::warning(this,
                             tr("Merge"),
                             tr("An error has occured while processing the request:\n\n%1").arg(e.description()));
    }
}
