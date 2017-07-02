#ifndef FORMEDITATTACHMENTS_H
#define FORMEDITATTACHMENTS_H

#include "camsegdialog.h"
#include <QAbstractListModel>

class QListView;

namespace KLib
{

    class AttachmentModel : public QAbstractListModel
    {
        Q_OBJECT

        public:
            AttachmentModel(QList<int>& _attachments, QObject* _parent = nullptr);

            int             rowCount(const QModelIndex& = QModelIndex()) const { return m_attachments.count(); }
            QVariant        data(const QModelIndex& _index, int _role) const;
            Qt::ItemFlags   flags(const QModelIndex &_index) const;
            bool            setData(const QModelIndex& _index, const QVariant& _value, int _role);

            void            addAttachment(int _id);
            void            removeAttachmentAt(int _index);

        private:
            QList<int>& m_attachments;
    };

    class FormEditAttachments : public CAMSEGDialog
    {
        Q_OBJECT

        public:
            FormEditAttachments(QList<int>& _attachments, QWidget* _parent = nullptr);

        public slots:
            void addAttachment();
            void removeCurrent();
            void displayCurrent();
            void saveCurrentAs();

        private:

            QListView* m_listWidget;

            AttachmentModel* m_model;
    };

}

#endif // FORMEDITATTACHMENTS_H
