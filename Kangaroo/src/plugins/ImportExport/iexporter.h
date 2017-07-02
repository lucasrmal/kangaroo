#ifndef IEXPORTER_H
#define IEXPORTER_H

#include <QString>
#include <QIcon>
#include "errors.h"

namespace KLib
{
    class ActionContainer;

    class IExporter
    {
        public:
            virtual QString fileType() const = 0;

            virtual QString description() const = 0;

            virtual QIcon icon() const = 0;

            virtual void import(const QString& _path, ErrorList& _errors) = 0;
    };

    class ExporterManager : public QObject
    {
        Q_OBJECT

        public:
            void addExporter(IExporter* _import);

            static ExporterManager* instance();

        private slots:
            void onActionActivated();

        private:
            ExporterManager();

            QList<IExporter*> m_exporters;
            ActionContainer* m_mnuExport;

            static ExporterManager* m_instance;
    };

}

#endif // IEXPORTER_H
