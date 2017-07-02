#ifndef IIMPORTER_H
#define IIMPORTER_H

#include <QString>
#include <QIcon>
#include "errors.h"

namespace KLib
{
    class ActionContainer;

    class IImporter
    {
        public:

            virtual QString fileType() const = 0;

            virtual QString description() const = 0;

            virtual QIcon icon() const = 0;

            virtual void import(const QString& _path, ErrorList& _errors) = 0;
    };

    class ImporterManager : public QObject
    {
        Q_OBJECT

        public:
            void addImporter(IImporter* _import);

            static ImporterManager* instance();

        private slots:
            void onActionActivated();

        private:
            ImporterManager();

            QList<IImporter*> m_importers;
            ActionContainer* m_mnuImport;

            static ImporterManager* m_instance;
    };
}

#endif // IIMPORTER_H
