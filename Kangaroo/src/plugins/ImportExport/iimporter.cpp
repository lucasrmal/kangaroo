#include "iimporter.h"
#include "formerrors.h"
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/command.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/controller/io.h>

#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

namespace KLib
{
ImporterManager* ImporterManager::m_instance = nullptr;

ImporterManager::ImporterManager()
{
    m_mnuImport = ActionManager::instance()->insertMenu(STD_MENUS::MENU_FILE,
                                                        "File.Export",
                                                        "File.Import",
                                                        tr("Import"),
                                                        Core::icon("document-import"));
}

ImporterManager* ImporterManager::instance()
{
    if (!m_instance)
        m_instance = new ImporterManager();

    return m_instance;
}

void ImporterManager::addImporter(IImporter* _import)
{
    QAction* a = m_mnuImport->menu()->addAction(_import->icon(), _import->description(), this, SLOT(onActionActivated()));
    a->setData(m_importers.count());
    m_importers.append(_import);
}

void ImporterManager::onActionActivated()
{
    QAction* act = static_cast<QAction*>(sender());

    if (act)
    {
        QString allFiles = tr("All files") + " (*.*)";
        QString filePath = QFileDialog::getOpenFileName(Core::instance()->mainWindow(),
                                                    tr("Import File"),
                                                    "",
                                                    m_importers[act->data().toInt()]->fileType() + ";;" + allFiles);

        if (!filePath.isEmpty())
        {
            if (!Core::instance()->beginLoadNew())
            {
                return;
            }

            ErrorList errors;

            try
            {

                m_importers[act->data().toInt()]->import(filePath, errors);
            }
            catch (IOException e)
            {
                errors << Error(Critical,
                                           tr("An error has occured while importing the file:\n\n%1").arg(e.description()));
            }

            bool continueLoading = true;

            if (errors.isEmpty())
            {
                //Show success dialog
                QMessageBox::information(Core::instance()->mainWindow(),
                                         tr("Import File"),
                                         tr("The file was imported successfully!"));
            }
            else
            {
                //Check if contains a critical error...
                for (const Error& e : errors)
                {
                    if (e.first == Critical)
                    {
                        continueLoading = false;
                        Core::instance()->unloadFile();
                        break;
                    }
                }

                //Show errors and warnings
                FormErrors::showErrorList(Core::instance()->mainWindow(),
                                          tr("Import File"),
                                          tr("Errors or warnings have occured while importing the file."),
                                          errors);
            }

            if (continueLoading)
            {
                Core::instance()->endLoadNew();
            }
        }

    }
}

}



