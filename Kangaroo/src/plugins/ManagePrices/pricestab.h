#ifndef PRICESTAB_H
#define PRICESTAB_H

#include <QWidget>
#include <QTreeView>

class QPushButton;

namespace KLib
{
class PriceController;
}

class PricesTab : public QWidget
{
    Q_OBJECT

    public:
        explicit PricesTab(QWidget *parent = 0);

    signals:

    public slots:
        void addRate();
        void removePair();

    private:
        QTreeView* m_treeView;
        KLib::PriceController* m_controller;

        QPushButton* m_btnAddRate;
        QPushButton* m_btnRemovePair;

};

#endif // PRICESTAB_H
