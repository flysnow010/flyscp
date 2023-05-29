#include "filecompresser.h"
#include "util/utils.h"
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QDebug>
QString CompressParam::volumeText() const
{
    if(volumeSize == 0)
        return QString();
    return QString("-v%1m").arg(volumeSize);
}

QString CompressParam::passwordText() const
{
    if(password.isEmpty())
        return password;
    return QString("-p%1").arg(password);
}

FileCompresser::FileCompresser(QObject *parent)
    : QObject(parent)
    , process(new QProcess(this))
    , currentIndex(0)
{
    connect(process, &QProcess::readyReadStandardOutput, this, [=](){
        while(process->canReadLine())
        {
            QString text = QString::fromLocal8Bit(process->readLine()).remove("\r\n");
            if(!text.isEmpty())
                emit progress(text);
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

bool FileCompresser::compress(QStringList const& fileNames,
              CompressParam const& param,
              QString const& targetFilePath)
{
    QString app = Utils::compressApp();
    process->setProgram(app);
    if(param.isSignle)
    {
        foreach(auto const& fileName, fileNames)
        {
            QStringList args;
            QFileInfo fileInfo(targetFilePath);
            QString targetFileName = fileInfo.dir()
                    .filePath(QFileInfo(fileName).baseName() + "." + fileInfo.suffix());
            args << "a" << targetFileName
                 << getFileNames(fileName, param);
            setArgs(args, param);
            argsList << args;
            qDebug()<< args.join(" ");
            targetFileNames << targetFileName;
        }
    }
    else
    {
        QStringList args;
        args << "a" << targetFilePath;
        foreach(auto const& fileName, fileNames)
        {
            args << getFileNames(fileName, param);
        }
        setArgs(args, param);
        argsList << args;
        qDebug()<< args.join(" ");
        targetFileNames << targetFilePath;
    }
    currentIndex = 0;
    process->setArguments(nextArgs());
    process->start();
    return true;
}

void FileCompresser::cancel()
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

void FileCompresser::setArgs(QStringList & args, CompressParam const& param)
{
    if(param.isRecursively)
        args << "-r";
    else
        args << "-r-";
    if(param.isMultiVolume)
        args << param.volumeText();
    if(param.isMoveFile)
        args <<  "-sdel";
    if(param.isCreateSFX)
        args <<  "-sfx";
    if(param.isEncryption)
        args << param.passwordText();
}

QStringList FileCompresser::getFileNames(QString const&fileName,
                      CompressParam const& param)
{
    QStringList newFileNames;
    QFileInfo fileInfo(fileName);
    if(fileInfo.isFile())
        newFileNames << fileName;
    else
    {
        if(param.isWithPath)
            newFileNames << fileName;
        else
            newFileNames << fileName + "/*";

    }
    return newFileNames;
}

QStringList FileCompresser::nextArgs() const
{
    return  argsList.at(currentIndex);
}

void FileCompresser::onError(QString const& errorText)
{
    emit error(errorText);
    emit finished();
}

QString FileCompresser::errorToText(int errorCode) const
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
