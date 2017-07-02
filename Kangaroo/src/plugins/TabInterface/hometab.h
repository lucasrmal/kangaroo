#ifndef HOMETAB_H
#define HOMETAB_H

#include <QScrollArea>
#include <QHash>
#include "ihomewidget.h"

class AccountTreeWidget;
class QVBoxLayout;

namespace KLib
{
    class Account;
}

class HomeTab : public QScrollArea
{
    Q_OBJECT

        typedef QPair<QString, IHomeWidget*> HomeWidgetPair;

    public:
        explicit HomeTab(QWidget* _parent = nullptr);
        ~HomeTab();

        static void registerHomeWidget(const HomeWidgetInfo& _infos);

        static const int MAX_WIDGET_HEIGHT = 500;

    public slots:
        void editWidgets();
        void removeWidget();
        void configureWidget();

    private:
        void loadWidgets();
        void saveWidgets();

        QVBoxLayout* m_layout;

        QList<HomeWidgetPair> m_displayedWidgets;

        static QHash<QString, HomeWidgetInfo> m_homeWidgets;

        friend class FormEditHomeTabWidgets;
};

#endif // HOMETAB_H
