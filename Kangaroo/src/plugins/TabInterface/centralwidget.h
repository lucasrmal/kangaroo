#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QSplitter>
#include <QMultiMap>

class QToolBox;
class HomeTab;
class WelcomeScreen;

class ISidePane
{
    public:
        virtual ~ISidePane() {}

        /**
         * @brief Title of the tab shown to the user
         */
        virtual QString title() const = 0;

        /**
         * @brief Weight of the pane. Heavier weights sink to the bottom.
         *
         * The 2 build-in panes have weight 0 (Accounts) and 50 (Reports).
         */
        virtual int weight() const = 0;

        /**
         * @brief Builds and returns the pane
         */
        virtual QWidget* buildPane(QWidget* _parent) const = 0;
};

class CentralWidget : public QSplitter
{
    Q_OBJECT

        explicit CentralWidget(QWidget* _parent = nullptr);

        void onLoad();
        void onUnload();

    public:
        ~CentralWidget() { saveSizes(); }

        static CentralWidget* instance();

        void registerSidePane(ISidePane* _pane);

    private:
        QToolBox*       m_sidePane;
        HomeTab*        m_homeTab;
        WelcomeScreen*  m_welcomeScreen;

        QMultiMap<int, ISidePane*> m_panes; //Ordered by weight

        static CentralWidget* m_instance;

        void saveSizes();

        friend class TabInterfacePlugin;

};

#endif // CENTRALWIDGET_H
