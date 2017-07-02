#ifndef TIEREDITOR_H
#define TIEREDITOR_H

#include <QAbstractTableModel>
#include <QTableView>
#include <QItemDelegate>
#include <KangarooLib/ui/dialogs/camsegdialog.h>
#include "rewardsprogram.h"

namespace TierColumn
{
    enum Columns
    {
        NAME = 0,
        RATE,
        NumColumns
    };
}

class TierController : public QAbstractTableModel
{
    Q_OBJECT

    public:
        TierController(QList<RewardTier>& _tiers, QObject *parent = 0);

        virtual         ~TierController() {}

        int             rowCount(const QModelIndex& _parent = QModelIndex()) const;
        int             columnCount(const QModelIndex& _parent = QModelIndex()) const;

        QVariant        data(const QModelIndex& _index, int _role) const;

        QVariant        headerData(int _section,
                                   Qt::Orientation _orientation,
                                   int _role = Qt::DisplayRole) const;

        Qt::ItemFlags   flags(const QModelIndex& _index) const;

        bool            setData(const QModelIndex& _index,
                                const QVariant& _value,
                                int _role = Qt::EditRole);

        bool            insertRows(int _row, int _count, const QModelIndex& _parent = QModelIndex());

        bool            removeRows(int _row, int _count, const QModelIndex& _parent = QModelIndex());

    public slots:
        void reload();

    private:
        bool rowIsEmpty(int _row);
        void ensureOneEmptyRow();

    private:
        QList<RewardTier>& m_tiers;
};

class TierEditorDelegate : public QItemDelegate
{
    Q_OBJECT

    public:
        TierEditorDelegate(QObject *parent = 0);
        virtual ~TierEditorDelegate() {}

        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

        void updateEditorGeometry(QWidget *editor,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const;
};

class TierEditor : public QTableView
{
    Q_OBJECT

    public:
        explicit TierEditor(QList<RewardTier>& _tiers, QWidget *parent = 0);

        QSize	sizeHint () const { return QSize(400, 230); }

    public slots:
        bool validate();
        void addRow();
        void removeCurrentRow();

        void reload();

    private:
        QList<RewardTier>& m_tiers;
        TierController* m_model;
        TierEditorDelegate m_delegate;
};

class FormTierEditor : public KLib::CAMSEGDialog
{
        Q_OBJECT

    public:
        FormTierEditor(QList<RewardTier>& _tiers, QWidget* _parent = 0);
        virtual ~FormTierEditor() {}

    public slots:
        void accept();

        void removeCurrentRow();

    private:
        TierEditor* m_tierEditor;

        QPushButton* m_btnInsert;
        QPushButton* m_btnRemove;
};

#endif // TIEREDITOR_H
