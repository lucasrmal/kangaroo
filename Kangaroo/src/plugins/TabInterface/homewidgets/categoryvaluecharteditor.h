#ifndef CATEGORYVALUECHARTEDITOR_H
#define CATEGORYVALUECHARTEDITOR_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>
#include <KangarooLib/controller/accountcontroller.h>
#include <QHash>

enum class CategoryValueType;
class QTreeView;

class CategoryEditorModel : public KLib::AccountController
{
        Q_OBJECT

    public:
        CategoryEditorModel(CategoryValueType _type, QObject* _parent = nullptr);

        QVariant        data(const QModelIndex& _index, int _role) const override;
        Qt::ItemFlags   flags(const QModelIndex& _index) const override;

        bool            setData(const QModelIndex& _index, const QVariant& _value, int _role) override;

        int             columnCount(const QModelIndex& _parent = QModelIndex()) const override { Q_UNUSED(_parent); return 1; }

    public slots:
        void            saveAll();

    private:
        void            loadData();

        void checkAllChildren(bool _check, KLib::Account* _parent, const QModelIndex& _index);
        void uncheckAllAncestors(KLib::Account* _child, const QModelIndex& _index);

        CategoryValueType           m_type;
        QHash<KLib::Account*, bool> m_checked;

};

class CategoryValueChartEditor : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:
        explicit CategoryValueChartEditor(CategoryValueType _type, QWidget* _parent = nullptr);

        QSize sizeHint() const override { return QSize(700, 900); }

    public slots:
        void accept();

    private:
        CategoryEditorModel* m_model;
        QTreeView* m_view;
};

#endif // CATEGORYVALUECHARTEDITOR_H
