#include "formerrors.h"

#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <KangarooLib/ui/core.h>

using namespace KLib;

FormErrors::FormErrors(const QString& _caption, const ErrorList& _errors, QWidget *parent)
    : CAMSEGDialog(DialogWithPicture, CloseButton, parent)
{
    QLabel* lblCaption = new QLabel(_caption, this);
    QListWidget* listErrors = new QListWidget(this);

    lblCaption->setWordWrap(true);

    for (auto e : _errors)
    {
        listErrors->addItem(new QListWidgetItem(Core::icon(e.first == Critical ? "dialog-error-16"
                                                                                           : "dialog-warning-16"),
                                                 e.second));
    }

    setPicture(Core::pixmap("document-import"));

    QVBoxLayout* layout = new QVBoxLayout(centralWidget());
    layout->addWidget(lblCaption);
    layout->addWidget(listErrors);
}

void FormErrors::showErrorList(QWidget* _parent,
                               const QString& _title,
                               const QString& _caption,
                               const KLib::ErrorList& _errors)
{
    FormErrors* e = new FormErrors(_caption,
                                   _errors,
                                   _parent);
    e->setBothTitles(_title);

    e->exec();
    delete e;
}

