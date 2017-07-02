#include "iexporter.h"
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

    ExporterManager* ExporterManager::m_instance = nullptr;

    ExporterManager::ExporterManager()
    {
        m_mnuExport = ActionManager::instance()->insertMenu(STD_MENUS::MENU_FILE,
                                                            "File.Save",
                                                            "File.Export",
                                                            tr("Export"),
                                                            Core::icon("document-export"));
        ActionManager::instance()->actionContainer("File")->menu()
                ->insertSeparator(ActionManager::instance()->command("File.Save")->action());
    }

    ExporterManager* ExporterManager::instance()
    {
        if (!m_instance)
            m_instance = new ExporterManager();

        return m_instance;
    }

    void ExporterManager::addExporter(IExporter* _import)
    {
        QAction* a = m_mnuExport->menu()->addAction(_import->icon(), _import->description(), this, SLOT(onActionActivated()));
        a->setData(m_exporters.count());
        m_exporters.append(_import);
    }

    void ExporterManager::onActionActivated()
    {
        QAction* act = static_cast<QAction*>(sender());

        if (act)
        {
            QString allFiles = tr("All files") + " (*.*)";
            QString filePath = QFileDialog::getOpenFileName(Core::instance()->mainWindow(),
                                                        tr("Export File"),
                                                        "",
                                                        m_exporters[act->data().toInt()]->fileType() + ";;" + allFiles);

            if (!filePath.isEmpty())
            {
                ErrorList errors;

                try
                {

                    m_exporters[act->data().toInt()]->import(filePath, errors);
                }
                catch (IOException e)
                {
                    errors << Error(Critical,
                                               tr("An error has occured while exporting the file:\n\n%1").arg(e.description()));
                }

                if (errors.isEmpty())
                {
                    //Show success dialog
                    QMessageBox::information(Core::instance()->mainWindow(),
                                             tr("Export File"),
                                             tr("The file was exported successfully!"));
                }
                else
                {
                    //Show errors and warnings
                    FormErrors::showErrorList(Core::instance()->mainWindow(),
                                              tr("Export File"),
                                              tr("Errors or warnings have occured while exporting the file."),
                                              errors);
                }
            }

        }
    }

}

