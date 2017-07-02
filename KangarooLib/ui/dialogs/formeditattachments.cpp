#include "formeditattachments.h"

#include <QListView>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include "../../model/modelexception.h"
#include "../../model/documentmanager.h"
#include "../core.h"

namespace KLib
{
    FormEditAttachments::FormEditAttachments(QList<int>& _attachments, QWidget* _parent) :
        CAMSEGDialog(DialogWithPicture, CloseButton, _parent),
        m_model(new AttachmentModel(_attachments, this))
    {
        m_listWidget = new QListView(this);
        m_listWidget->setModel(m_model);

        setCentralWidget(m_listWidget);
        setBothTitles(tr("Transaction Attachments"));
        setPicture(Core::pixmap("attachment"));

        m_listWidget->setIconSize(QSize(48, 48));

        //Buttons
        QPushButton* btnAdd     = new QPushButton(Core::icon("add"), tr("&Add"), this);
        QPushButton* btnRemove  = new QPushButton(Core::icon("trash"), tr("&Delete"), this);
        QPushButton* btnView    = new QPushButton(Core::icon("document-preview"), tr("&View"), this);
        QPushButton* btnSaveAs  = new QPushButton(Core::icon("save-as"), tr("&Save As"), this);

        addButton(btnSaveAs, 0, AtLeft);
        addButton(btnView,   0, AtLeft);
        addButton(btnRemove, 0, AtLeft);
        addButton(btnAdd,    0, AtLeft); //Reverse order: O(n).

        connect(btnAdd,     &QPushButton::clicked, this, &FormEditAttachments::addAttachment);
        connect(btnRemove,  &QPushButton::clicked, this, &FormEditAttachments::removeCurrent);
        connect(btnView,    &QPushButton::clicked, this, &FormEditAttachments::displayCurrent);
        connect(btnSaveAs,  &QPushButton::clicked, this, &FormEditAttachments::saveCurrentAs);
    }

    void FormEditAttachments::addAttachment()
    {
        //Display open file dialog

        QString file = QFileDialog::getOpenFileName(this,
                                                    tr("Add Attachment"),
                                                    QDir::homePath(),
                                                    tr("All Files (*.*)"));

        if (!file.isEmpty())
        {
            try
            {
                Document* d = DocumentManager::instance()->add(file);
                m_model->addAttachment(d->id);
            }
            catch (ModelException e)
            {
                QMessageBox::warning(this, tr("View Document"), tr("An error has occured while "
                                                                   "processing the document: %1").arg(e.description()));
            }
        }
    }

    void FormEditAttachments::removeCurrent()
    {
        //Remove the attachment
        if (m_listWidget->currentIndex().isValid())
        {
            m_model->removeAttachmentAt(m_listWidget->currentIndex().row());
        }
    }

    void FormEditAttachments::displayCurrent()
    {
        if (m_listWidget->currentIndex().isValid())
        {
            try
            {
                DocumentManager::instance()->display(m_listWidget->currentIndex().data(Qt::UserRole).toInt());
            }
            catch (ModelException e)
            {
                QMessageBox::warning(this, tr("View Document"), tr("An error has occured while "
                                                                   "processing the document: %1").arg(e.description()));
            }
        }
    }

    void FormEditAttachments::saveCurrentAs()
    {
        if (m_listWidget->currentIndex().isValid())
        {
            try
            {
                const Document* d = DocumentManager::instance()->get(m_listWidget->currentIndex().data(Qt::UserRole).toInt());

                //Display file save dialog
                QString file = QFileDialog::getSaveFileName(this,
                                                            tr("Save Document As"),
                                                            QDir::homePath(),
                                                            d->mime.filterString());

                if (!file.isEmpty())
                {
                    QFile f(file);

                    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
                    {
                        QMessageBox::warning(this,
                                             tr("Save Document As"),
                                             tr("An error has occured while saving the document:\n\n%1").arg(f.errorString()));
                        return;
                    }

                    f.write(d->contents);
                    f.close();
                }
            }
            catch (...) {}
        }
    }

    //////////////////////////////////////////// MODEL ////////////////////////////////////////////

    AttachmentModel::AttachmentModel(QList<int>& _attachments, QObject* _parent) :
        QAbstractListModel(_parent),
        m_attachments(_attachments)
    {
    }

    QVariant AttachmentModel::data(const QModelIndex& _index, int _role) const
    {
        if (!_index.isValid()
            || _index.row() >= m_attachments.count()
            || _index.column() != 0)
        {
            return QVariant();
        }

        try
        {
            const Document* d = DocumentManager::instance()->get(m_attachments[_index.row()]);

            switch (_role)
            {
            case Qt::DisplayRole:
            case Qt::EditRole:
                return d->memo;

            case Qt::DecorationRole:
                return QIcon::fromTheme(d->mime.iconName());

            case Qt::UserRole:
                return d->id;
            }
        }
        catch (...) {}

        return QVariant();
    }

    Qt::ItemFlags AttachmentModel::flags(const QModelIndex& _index) const
    {
        return QAbstractListModel::flags(_index) | Qt::ItemIsEditable;
    }

    bool AttachmentModel::setData(const QModelIndex& _index, const QVariant& _value, int _role)
    {
        //Allowed to change the memo of the document

        if (_index.isValid()
            && _index.row() < m_attachments.count()
            && _index.column() == 0
            && !_value.toString().isEmpty()
            && _role == Qt::EditRole)
        {
            try
            {
                DocumentManager::instance()->setMemo(m_attachments[_index.row()], _value.toString());
                emit dataChanged(_index, _index);
                return true;
            }
            catch (...) { return false; }
        }
        else
        {
            return false;
        }
    }

    void AttachmentModel::addAttachment(int _id)
    {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_attachments.append(_id);
        endInsertRows();
    }

    void AttachmentModel::removeAttachmentAt(int _index)
    {
        if (_index != -1)
        {
            beginRemoveRows(QModelIndex(), _index, _index);
            m_attachments.removeAt(_index);
            endRemoveRows();
        }
    }

}

