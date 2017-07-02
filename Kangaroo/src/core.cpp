#include "core.h"
#include "mainwindow.h"
#include "actionmanager.h"
#include "frmabout.h"
#include "frmwelcome.h"
#include "global.h"
#include "SCMPlugin.h"

#include <libCAMSEG/campaign.h>

#include <QSettings>
#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <QPluginLoader>
#include <QDir>
#include <QMessageBox>

using namespace CAMSEG;

namespace SCMApplication {

    ISCMApplication* ISCMApplication::m_instance = NULL;

    ISCMApplication::ISCMApplication() : QObject()
    {
        //Create childs
        m_mainWindow = new MainWindow();
        m_actionManager = new ActionManager(m_mainWindow->menuBar(), this);

        //Set campaign to NULL
        m_campaign = NULL;

        //Load plugins
        loadPlugins();

        //Connect frmWelcome signals
        connect(m_mainWindow->m_welcomeScreen, SIGNAL(showAbout()), this, SLOT(showAboutDialog()));
    }
    
    ISCMApplication::~ISCMApplication()
    {
        delete m_mainWindow;
    }

    ISCMApplication* ISCMApplication::instance()
    {
        if (m_instance == NULL)
        {
            m_instance = new ISCMApplication;
            m_instance->initializePlugins();
        }

        return m_instance;
    }

    CentralWidget* ISCMApplication::centralWidget() const
    {
        return m_mainWindow->m_centralWidget;
    }

    bool ISCMApplication::closeCampaign()
    {

    }

    bool ISCMApplication::newCampaign()
    {

    }

    bool ISCMApplication::openCampaign(const QString & p_path)
    {

    }

    bool ISCMApplication::openCampaign(const CAMSEG::StorageFormat p_storageFormat, const QString p_path)
    {

    }

    void ISCMApplication::showHelp()
    {
        QSettings settings;
        QString language = settings.value("General/Language").toString();
        QString pathFile = QString("help_%1.pdf").arg(language);

        if ( ! QFile::exists(PATH_HELP + pathFile))
        {
            pathFile = QString("help_en.pdf");
        }

        QDesktopServices::openUrl(QUrl(QString("file://%1").arg(PATH_HELP + pathFile), QUrl::TolerantMode));
    }

    void ISCMApplication::showAboutDialog()
    {
        frmAbout* dlAbout = new frmAbout(m_mainWindow);
        dlAbout->exec();
        delete dlAbout;
    }

    void ISCMApplication::showAboutPlugins()
    {
    }

    void ISCMApplication::loadPlugins()
    {
        //Load static plugins
        foreach (QObject *plugin, QPluginLoader::staticInstances())
            loadPlugin(plugin);

        //Load dynamic plugins
        QDir pluginsDir(PATH_PLUGINS);

        foreach (QString fileName, pluginsDir.entryList(QDir::Files))
        {
             QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
             QObject *plugin = loader.instance();

             if (plugin)
             {
                 loadPlugin(plugin);
             }
        }
    }

    void ISCMApplication::initializePlugins()
    {
        //Initialize plugins
        QStringList args;
        QString errorMessage;
        foreach (SCMPlugin* plugin, m_plugins)
        {
            if ( !plugin->initialize(args, errorMessage))
            {
                QMessageBox::information(m_mainWindow, tr("Load Plugins"), tr("An error has occured while loading plugin %1 :\n%2").arg(plugin->name()).arg(errorMessage));
                QApplication::exit(0);
            }
        }
    }

    void ISCMApplication::loadPlugin(QObject* p_plugin)
    {
        SCMPlugin* plugin = qobject_cast<SCMPlugin*>(p_plugin);
        if (plugin)
            m_plugins << plugin;
    }

}
