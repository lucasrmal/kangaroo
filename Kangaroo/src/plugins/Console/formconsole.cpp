#include "formconsole.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/controller/scriptengine.h>

#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QScriptEngine>

using namespace KLib;

FormConsole::FormConsole(QWidget *parent) :
    CAMSEGDialog(DialogWithPicture, CloseButton, parent)
{
    loadUI();

    m_lastPath = QDir::homePath();

    txtResult->setFocus();
}

void FormConsole::loadUI()
{
    setTitle(tr("Script Console"));
    setWindowTitle(title());
    setPicture(Core::pixmap("console"));

    txtQuery = new QPlainTextEdit();
    txtResult = new QPlainTextEdit();
    btnExecute = new QPushButton(Core::icon("execute"), tr("&Execute"));
    btnClear = new QPushButton(Core::icon("clear"), tr("C&lear"));
    btnLoad = new QPushButton(Core::icon("open"), tr("&Open Script"));
    btnSave = new QPushButton(Core::icon("save"), tr("&Save Script"));

    grbQuery = new QGroupBox(tr("Query"));
    grbOutput = new QGroupBox(tr("Output"));

    txtResult->setReadOnly(true);

    QPalette outputPal = txtResult->palette();
    outputPal.setColor(QPalette::Base, Qt::black);
    outputPal.setColor(QPalette::Text, Qt::white);
    txtResult->setPalette(outputPal);

    //Fonts
    QFont monoFont("mono");
    monoFont.setStyleHint(QFont::TypeWriter);
    txtResult->setFont(monoFont);
    txtQuery->setFont(monoFont);

    QSplitter* splitter = new QSplitter();
    QVBoxLayout* queryLayout = new QVBoxLayout(grbQuery);
    QVBoxLayout* outputLayout = new QVBoxLayout(grbOutput);
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    buttonLayout->addStretch(10);
    buttonLayout->addWidget(btnExecute);
    buttonLayout->addWidget(btnClear);
    buttonLayout->addWidget(btnLoad);
    buttonLayout->addWidget(btnSave);

    queryLayout->addWidget(txtQuery);
    queryLayout->addLayout(buttonLayout);

    outputLayout->addWidget(txtResult);

    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(grbQuery);
    splitter->addWidget(grbOutput);

    setCentralWidget(splitter);

    //Connect signals
    connect(btnExecute, SIGNAL(clicked()), this, SLOT(executeQuery()));
    connect(btnClear, SIGNAL(clicked()), this, SLOT(clear()));
    connect(btnLoad, SIGNAL(clicked()), this, SLOT(load()));
    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
}

void FormConsole::executeQuery()
{
    QString result;
    ScriptEngine::instance()->execute(txtQuery->toPlainText(), result);
    txtResult->setPlainText(result);
}

void FormConsole::clear()
{
    txtResult->clear();
    txtQuery->clear();
    txtQuery->setFocus();
}

void FormConsole::load()
{
    //Get the path
    QString ecmaFile = tr("ECMAScript File") + " (*.es)";
    QString allFiles = tr("All files") + " (*.*)";
    QString path = QFileDialog::getOpenFileName(this, tr("Open Script"), m_lastPath, ecmaFile + ";;" + allFiles);

    if (path.isEmpty())
        return;

    m_lastPath = path;

    //Open the file
    QFile file(path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        txtQuery->setPlainText(in.readAll());
        file.close();
    }
    else
    {
        QMessageBox::warning(this,
                             tr("Open Script"),
                             tr("An error has occured while opening the file :\n%1").arg(file.errorString()));
    }
}

void FormConsole::save()
{
    //Get the path
    QString ecmaFile = tr("ECMAScript File") + " (*.es)";
    QString path = QFileDialog::getSaveFileName(this, tr("Save Script As"), m_lastPath, ecmaFile);

    if (path.isEmpty())
        return;

    if ( ! path.endsWith(".es"))
        path += ".es";

    m_lastPath = path;

    if (QFile::exists(path))
    {
        if (QMessageBox::question(this,
                                  tr("Save Script As"),
                                  tr("A file named %1 already exists. Replace it?").arg(path),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
            return;
    }

    //Save the file
    QFile file(path);

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QTextStream out(&file);
        out << txtQuery->toPlainText();
        file.close();
    }
    else
    {
        QMessageBox::warning(this,
                             tr("Save Script As"),
                             tr("An error has occured while saving the file :\n%1").arg(file.errorString()));
    }
}

void FormConsole::close()
{
    clear();
    done(QDialog::Accepted);
}

