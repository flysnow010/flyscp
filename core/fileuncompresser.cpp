#include "fileuncompresser.h"
#include "winshell.h"
#include "util/utils.h"

#include <QStringList>
#include <QProcess>
#include <QDebug>

QString UncompressParam::overwriteMode() const
{
    if(isWithPath)
        return QString();

    if(mode == OverWrite)
        return QString(" -aoa");
    else if(mode == Skip)
        return QString(" -aos");
    else if(mode == AutoRename)
        return QString(" -aot");
    return QString();
}

FileUncompresser::FileUncompresser(QObject *parent)
    : QObject(parent)
    , process(new QProcess(this))
    , mode(Uncompress)
    , isEncrypted_(false)
{
    connect(process, &QProcess::readyReadStandardOutput, this, [=](){
        while(process->canReadLine())
        {
            QString text = QString::fromUtf8(process->readLine()).remove("\r\n");
            if(!text.isEmpty())
            {
                if(mode == Uncompress)
                    emit progress(text);
                else if(mode == CheckEncrypt)
                {
                    if(text.contains("Encrypted = +"))
                        isEncrypted_ = true;
                }
            }
        }
    });
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [=](int exitCode, QProcess::ExitStatus exitStatus) {
        if(exitStatus != QProcess::ExitStatus::NormalExit)
            onError("Program is crash exit");
        else {
            if(exitCode != 0)
                onError(errorToText(exitCode));
            else {
                if(++currentIndex == argsList.size())
                    emit finished();
                else {
                    process->setArguments(nextArgs());
                    process->start();
                }
            }
        }
    });
}

bool FileUncompresser::uncompress(QStringList const& fileNames,
                UncompressParam const& param,
                QString const& targetFilePath)
{
    QString app = Utils::compressApp();
    mode = Uncompress;
    process->setProgram(app);
    foreach(auto const& fileName, fileNames)
    {
        QStringList args;
        args << (param.isWithPath ? "x" : "e")
             << fileName;

        if(!param.isCreateDir)
            args << "-o" + targetFilePath;
        else
        {
            QDir dir(targetFilePath);
            QString pathName = QFileInfo(fileName).baseName();
            dir.mkdir(pathName);
            args <<  "-o" + dir.filePath(pathName);
        }
        args << param.filter;
        if(!param.isWithPath)
            args <<  param.overwriteMode() << "-y";
        argsList << args;
    }
    currentIndex = 0;
    process->setArguments(nextArgs());
    process->start();
    return true;
}

bool FileUncompresser::isEncrypted(QString const& fileName)
{
    QString app = Utils::compressApp();
    process->setProgram(app);
    QStringList args;
    args << "l -slt" << fileName;
    argsList << args;
    mode = CheckEncrypt;
    currentIndex = 0;
    process->setArguments(nextArgs());
    process->start();
    process->waitForFinished();
    return isEncrypted_;
}

void FileUncompresser::cancel()
{
    if (process->state() == QProcess::NotRunning)
    {
        emit finished();
        return;
    }
    process->disconnect();
    process->kill();
    process->waitForFinished(1000);
    onError("User stopped the process");
}

QStringList FileUncompresser::nextArgs() const
{
    return  argsList.at(currentIndex);
}

void FileUncompresser::onError(QString const& errorText)
{
    emit error(errorText);
    emit finished();
}

QString FileUncompresser::errorToText(int errorCode) const
{
    if(errorCode == 1)
        return QString("Warning");
    else if(errorCode == 2)
        return QString("Fatal error");
    else if(errorCode == 7)
        return QString("Command line error");
    else if(errorCode == 8)
        return QString("Not enough memory for operation");
    else if(errorCode == 255)
        return QString("User stopped the process");
    return QString();
}
