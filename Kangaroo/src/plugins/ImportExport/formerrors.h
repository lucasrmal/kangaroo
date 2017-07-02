#ifndef FORMERRORS_H
#define FORMERRORS_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>
#include "iimporter.h"
#include "errors.h"

class FormErrors : public KLib::CAMSEGDialog
{
    Q_OBJECT

    explicit FormErrors(const QString& _caption, const KLib::ErrorList& _errors, QWidget *parent = nullptr);

    public:
        static void showErrorList(QWidget* _parent,
                                  const QString& _title,
                                  const QString& _caption,
                                  const KLib::ErrorList& _errors);

        QSize sizeHint() const { return QSize(650, 400); }
};

#endif // FORMERRORS_H
