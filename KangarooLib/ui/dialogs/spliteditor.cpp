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

#include "spliteditor.h"
#include "../core.h"
#include "../widgets/splitswidget.h"
#include "../../model/account.h"
#include "../../model/security.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

namespace KLib
{

    SplitEditor::SplitEditor(QList<Transaction::Split>& _splits, bool _readOnly, QWidget *parent) :
        CAMSEGDialog(DialogWithPicture, _readOnly ? CloseButton : OkCancelButtons, parent),
        m_readOnly(_readOnly)
    {
        setBothTitles(_readOnly ? tr("View Splits") : tr("Edit Splits"));
        setPicture(Core::pixmap("split"));

        m_splitsWidget = new SplitsWidget(_splits, _readOnly, this);
        m_lblImbalances = new QLabel(this);

        QVBoxLayout* layout = new QVBoxLayout(centralWidget());
        layout->addWidget(m_splitsWidget);
        layout->addWidget(m_lblImbalances);
        layout->setMargin(0);

        if (!_readOnly)
        {
            m_btnInsert = new QPushButton(Core::icon("list-add"), "", this);
            m_btnRemove = new QPushButton(Core::icon("list-remove"), "", this);

            connect(m_btnInsert, SIGNAL(clicked()), m_splitsWidget, SLOT(addRow()));
            connect(m_btnRemove, SIGNAL(clicked()), m_splitsWidget, SLOT(removeCurrentRow()));

            addButton(m_btnInsert, 0, CAMSEGDialog::AtLeft);
            addButton(m_btnRemove, 1, CAMSEGDialog::AtLeft);
        }

        connect(m_splitsWidget, SIGNAL(amountChanged()),
                this, SLOT(updateImbalances()));

        updateImbalances();
    }

    void SplitEditor::updateImbalances()
    {
        m_lblImbalances->setText(tr("Imbalances:%1").arg(Transaction::splitsImbalances(m_splitsWidget->validSplits())));
    }

    void SplitEditor::accept()
    {
        if (m_readOnly || m_splitsWidget->validate())
        {
            done(QDialog::Accepted);
        }
    }

}
