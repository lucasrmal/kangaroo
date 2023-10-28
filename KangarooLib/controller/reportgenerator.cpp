#include "reportgenerator.h"
#include "io.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

namespace KLib
{
    //const QString ReportGenerator::COMMAND_TEX_TO_PDF = "pdflatex";
    const QString ReportGenerator::TAG_BEGIN_CODE = "<?";
    const QString ReportGenerator::TAG_END_CODE = "?>";

    QString ReportGenerator::generateHtml(const QString& _inFile, const QString& _additionalStatement)
    {
        QString fullCode;
        QString currentStatement;
        bool inCode = false;
        QString buffer;
        int lineNo = 1;

        QFile file(_inFile);

        if (!file.open(QIODevice::ReadOnly))
        {
            throw IOException(QObject::tr("Unable to open the report file: %1").arg(file.errorString()));
        }

        QTextStream stream(&file);

        //Parse the text and transform all blocks not in code tags into print statements.
        while (!stream.atEnd())
        {
            buffer += stream.read(1);

            if (buffer.length() == 1)
                continue;   //Wait to read at least 2 characters

            if (buffer == TAG_BEGIN_CODE)
            {
                if (inCode)
                {
                    throw IOException(QObject::tr("Parse error: Unexpected begin tag at line %1.").arg(lineNo));
                }

                currentStatement.replace("\\", "\\\\");
                currentStatement.replace("\"", "\\\"");
                currentStatement.replace("\n", "\\n\" + \n \"");   //Replace end of lines with " + \n "
                fullCode += "print(\"" + currentStatement + "\");\n";
                currentStatement.clear();
                inCode = true;
                buffer.clear();
            }
            else if (buffer == TAG_END_CODE)
            {
                if (!inCode)
                {
                    throw IOException(QObject::tr("Parse error: Unexpected end tag at line %1.").arg(lineNo));
                }

                fullCode += currentStatement + "\n";
                currentStatement.clear();
                inCode = false;
                buffer.clear();
            }
            else    //Add the previous character to the statement, keep the second in the buffer.
            {
                if (buffer[0] == '\n') ++lineNo;

                currentStatement += buffer[0];
                buffer = buffer[1];
            }
        }

        currentStatement += buffer; //Add the last statement to the buffer.

        //Add the last statement to the code
        if (inCode)
        {
            fullCode += currentStatement + "\n";
        }
        else
        {
            currentStatement.replace("\\", "\\\\");
            currentStatement.replace("\"", "\\\"");
            currentStatement.replace("\n", "\\n\" + \n \"");   //Replace end of lines with " + \n "
            fullCode += "print(\"" + currentStatement + "\");\n";
        }

        file.close();

        //qDebug() << fullCode;

        //Generate the HTML code
        QString output;

//        if (!ScriptEngine::instance()->execute(_additionalStatement + fullCode, output))
//        {
//            throw IOException(QObject::tr("An error has occured while executing the script:\n%1").arg(output));
//        }

        //Return the HTML code
        return output;

//        file.setFileName(_outFile);

//        if (!file.open(QIODevice::WriteOnly))
//        {
//            throw IOException(QObject::tr("Unable to open the output file: %1").arg(file.errorString()));
//        }

//        stream << output;
//        file.close();
    }

//    bool ReportGenerator::generatePDF(const QString& _inFile, const QString& _dir)
//    {
//        QDir dir(_dir);
//        QFileInfo info(_inFile);

//        system(QString("%1 -output-directory=%2 %3").arg(COMMAND_TEX_TO_PDF)
//                                                    .arg(dir.absolutePath())
//                                                    .arg(info.fileName()).toLatin1().data());

//        time_t t0 = time(NULL);
//        while(static_cast<int>(time(NULL)-t0) < 2) {}

//        //qDebug() << dir.absolutePath() + "/" + info.baseName() + ".pdf";

//        return (QFileInfo(dir.absolutePath() + "/" + info.baseName() + ".pdf").exists());
//    }
}


