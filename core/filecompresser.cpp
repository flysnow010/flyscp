#include "filecompresser.h"
#include "util/utils.h"
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QTimer>

FileCompresser::FileCompresser(QObject *parent)
    : QObject(parent)
    , process(new QProcess(this))
    , compressCount(0)
{
    connect(process, &QProcess::readyReadStandardOutput, this, [=](){
        while(process->canReadLine())
        {
            QString text = QString::fromUtf8(process->readLine());
            qDebug() << text;
        }
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [=](int exitCode, QProcess::ExitStatus exitStatus) {
        if(exitStatus != QProcess::ExitStatus::NormalExit)
            emit error("Program is crash exit");
        else
        {
            if(exitCode == 0 && --compressCount == 0)
                emit finished();
            else
                emit error(errorToText(exitCode));
        }
    });
}

void FileCompresser::onDataReady()
{
    QString text = QString::fromUtf8(process->readAllStandardOutput());
    qDebug() << text;
}

bool FileCompresser::compress(QStringList const& fileNames,
              CompressParam const& param,
              QString const& targetFilePath)
{
    QString app = Utils::compressApp();
    if(param.isSignle)
    {
        compressCount = fileNames.size();
        foreach(auto const& fileName, fileNames)
        {
            QString srcFileName;

            if(QFileInfo(fileName).isDir() && !param.isWithPath)
                srcFileName = fileName + "/*";
            else
                srcFileName = fileName;
            QFileInfo fileInfo(targetFilePath);
            QString dstFileName = fileInfo.dir()
                    .filePath(QFileInfo(fileName).baseName() + "." + fileInfo.suffix());

            QString params = QString("a %1 %2")
                    .arg(dstFileName, srcFileName);
            if(param.isMoveFile)
                params += " -sdel";

            process->start(app + " " + params);
        }
    }
    else
    {
        QStringList args;
        args << "a" << targetFilePath;
        foreach(auto const& fileName, fileNames)
        {
            if(QFileInfo(fileName).isDir() && !param.isWithPath)
                args << (fileName + "/*");
            else
                args << fileName;
        }
        if(param.isMoveFile)
            args << "-sdel";
        compressCount = 1;
        process->setProgram(app);
        process->setArguments(args);
        QTimer::singleShot(0, this, [=](){
            process->start();
        });
    }
    return true;
}

void FileCompresser::cancel()
{
    process->kill();
    emit finished();
}

bool FileCompresser::isFinished() const
{
    return process->state() == QProcess::NotRunning;
}

QString FileCompresser::errorToText(int errorCode)
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
