#include "filecompresser.h"
#include "filename.h"
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

bool CompressParam::isMultiSuffix() const
{
    return suffix.startsWith(".tar.");
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
    process->setProgram(Utils::compressApp());
    if(param.isSignle)
    {
        foreach(auto const& fileName, fileNames)
        {
            QString newFileName = getNewFileName(targetFilePath, fileName);
            QString targetFileName;
            QStringList args;

            args << "a";
            if(!param.isMultiSuffix())
                args << newFileName;
            else
            {
                targetFileName = getFileName(newFileName, ".tar");
                args << targetFileName;
            }
            args << getFileNames(fileName, param);
            setArgs(args, param);
            addArgs(args);
            if(param.isMultiSuffix())
            {
                args = QStringList() << "a" << newFileName << targetFileName << "-sdel";
                addArgs(args);
            }
            targetFileNames << newFileName;
        }
    }
    else
    {
        QStringList args;
        QString targetFileName;

        args << "a";
        if(!param.isMultiSuffix())
            args << targetFilePath;
        else
        {
            targetFileName = getFileName(targetFilePath, ".tar");
            args << targetFileName;
        }
        foreach(auto const& fileName, fileNames)
        {
            args << getFileNames(fileName, param);
        }
        setArgs(args, param);
        addArgs(args);
        if(param.isMultiSuffix())
        {
            args = QStringList() << "a" << targetFilePath << targetFileName << "-sdel";
            addArgs(args);
        }
        targetFileNames << targetFilePath;
    }

    currentIndex = 0;
    process->setArguments(nextArgs());
    process->start();

    return true;
}

bool FileCompresser::update(QStringList const& fileNames,
                            QString const& archiveFileName,
                            bool isWaitForFinished)
{
    QStringList args;

    args << "a" << archiveFileName;
    foreach(auto fileName, fileNames)
        args << fileName;
    addArgs(args);

    currentIndex = 0;
    process->setProgram(Utils::compressApp());
    process->setArguments(nextArgs());
    process->start();
    if(isWaitForFinished)
        process->waitForFinished();

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

    foreach(auto const& fileName, targetFileNames)
        QFile::remove(fileName);
}

void FileCompresser::setArgs(QStringList & args,
                             CompressParam const& param)
{
    if(param.isMultiVolume)
        args << param.volumeText();
    if(param.isMoveFile)
        args <<  "-sdel";
    if(param.isEncryption)
        args << param.passwordText();
    if(param.isCreateSFX)
    {
        if(param.isGuiSFX)
             args <<  "-sfx7z.sfx";
        else
            args <<  "-sfx7zCon.sfx";
        args << "-t7z";
    }
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
        if(param.isRecursively)
        {
            if(param.isWithPath)
                newFileNames << "-spf1";
            newFileNames << "-r" << fileName + "/" +  param.filter;
        }
        else
        {
            QDir dir(fileName);
            QFileInfoList fileInfos = dir.entryInfoList();
            foreach(auto const& info, fileInfos)//param.filter
            {
                if(!info.isFile())
                    continue;
                newFileNames << dir.filePath(info.fileName());
            }
            if(param.isWithPath)
                newFileNames << "-spf1";
        }

    }
    return newFileNames;
}

QString FileCompresser::getFileName(QString const& fileName,
                                    QString const& newSuffix)
{
    QFileInfo fileInfo(fileName);
    return fileInfo.dir().filePath(fileInfo.baseName() + newSuffix);
}

QString FileCompresser::getNewFileName(QString const& targetFilePath,
                                       QString const& fileName)
{
    QFileInfo fileInfo(targetFilePath);
    return fileInfo.dir().filePath(
                QFileInfo(fileName).baseName() + "." + fileInfo.completeSuffix());
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

void FileCompresser::addArgs(QStringList & args)
{
    qDebug()<< args.join(" ");
    argsList << args;
}
