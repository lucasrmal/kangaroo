#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QVector>
#include <libCAMSEG/campaign.h>

class SCMPlugin;

namespace SCMApplication {

    class MainWindow;
    class ActionManager;
    class CentralWidget;

    class ISCMApplication : public QObject
    {
        Q_OBJECT
                                ISCMApplication();

        public:
                                ~ISCMApplication();

     static ISCMApplication*    instance();

            MainWindow*         mainWindow() const          { return m_mainWindow; }
            ActionManager*      actionManager() const       { return m_actionManager; }
            CAMSEG::Campaign*   campaign() const            { return m_campaign; }
            CentralWidget*      centralWidget() const;

            bool                isCampaignOpened() const    { return m_campaign == NULL; }

        public slots:

            bool                closeCampaign();
            bool                newCampaign();
            bool                openCampaign(const QString & p_path);
            bool                openCampaign(const CAMSEG::StorageFormat p_storageFormat, const QString p_path = "");

            void                showHelp();
            void                showAboutDialog();
            void                showAboutPlugins();

        private:
            void                loadPlugins();
            void                initializePlugins();
            void                loadPlugin(QObject* p_plugin);

     static ISCMApplication*    m_instance;

            MainWindow*         m_mainWindow;
            ActionManager*      m_actionManager;

            CAMSEG::Campaign*   m_campaign;

            QVector<SCMPlugin*> m_plugins;
    };
}

#endif // CORE_H
