#ifndef REWARDSPROGRAMCONTROLLER_H
#define REWARDSPROGRAMCONTROLLER_H

#include <QAbstractTableModel>
#include <QItemDelegate>

class QSortFilterProxyModel;

class RewardsProgramManager;
class RewardsProgram;

namespace RewardsProgramColumn
{
    enum Columns
    {
        NAME,
        CURRENCY,
        NumColumns
    };
}

class RewardsProgramController : public QAbstractTableModel
{
    Q_OBJECT

    public:
        explicit RewardsProgramController(QObject *parent = 0);

        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const;

        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value,
                     int role = Qt::EditRole);

        static QSortFilterProxyModel* sortProxy(QObject *_parent);

    private slots:
        void onProgramAdded(RewardsProgram* _p);
        void onProgramModified(RewardsProgram* _p);
        void onProgramRemoved(RewardsProgram* _p);

    private:
        RewardsProgramManager* m_manager;
        QList<RewardsProgram*> m_sorted;


};

class RewardsProgramDelegate : public QItemDelegate
{
    Q_OBJECT

    public:
        RewardsProgramDelegate(QObject* _parent = 0);

        QWidget* createEditor(QWidget* _parent, const QStyleOptionViewItem& _option, const QModelIndex& _index) const;

        void setEditorData(QWidget* _editor, const QModelIndex& _index) const;
        void setModelData(QWidget* _editor, QAbstractItemModel* _model, const QModelIndex &_index) const;
        void updateEditorGeometry(QWidget* _editor,
                                  const QStyleOptionViewItem& _option,
                                  const QModelIndex& _index) const;
};

#endif // REWARDSPROGRAMCONTROLLER_H
