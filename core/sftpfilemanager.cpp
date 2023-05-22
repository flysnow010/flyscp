#include "sftpfilemanager.h"
#include "sftp/sftpsession.h"
#include "filename.h"
#include <QDebug>
#include <QFileInfo>

SFtpFileManager::SFtpFileManager(SFtpSession* sftp, QObject *parent)
    : RemoteFileManager(parent)
    , sftpSession(sftp)
{

}

void SFtpFileManager::doUploadFiles(FileNames const& fileNames)
{
    FileNames newFileNames;
    FileNames::MakeFileNames(fileNames, newFileNames);
    for(int i = 0; i < newFileNames.size(); i++)
    {
        if(singled())
            break;
        qDebug() << newFileNames[i].src << "->" <<newFileNames[i].dst;
        emit totalProgress(newFileNames[i].src, newFileNames[i].dst, newFileNames.size(), i);
        qDebug() << QFileInfo(newFileNames[i].dst).path();
        sftpSession->mkdir(QFileInfo(newFileNames[i].dst).path().toStdString());
        QFileInfo(newFileNames[i].dst).path();
        uploadFile(newFileNames[i].src, newFileNames[i].dst);
        emit totalProgress(newFileNames[i].src, newFileNames[i].dst, newFileNames.size(), i + 1);
    }
    emit finished();
}

void SFtpFileManager::doDownloadFiles(FileNames const& fileNames)
{
    FileNames newFileNames;
    FileNames::MakeFileNames(fileNames, newFileNames);
    for(int i = 0; i < newFileNames.size(); i++)
    {
        if(singled())
            break;
        emit totalProgress(newFileNames[i].src, newFileNames[i].dst, newFileNames.size(), i);
        //mkdir
        downloadFile(newFileNames[i].src, newFileNames[i].dst);
        emit totalProgress(newFileNames[i].src, newFileNames[i].dst, newFileNames.size(), i + 1);
    }
    emit finished();
}

void SFtpFileManager::doDelereFiles(QStringList const& remoteFileNames, QString const& filePath, bool isDst)
{
    QStringList newFileNames;
    FileNames::MakeFileNames(remoteFileNames, newFileNames);
    for(int i = 0; i < newFileNames.size(); i++)
    {
        if(singled())
            break;

        emit totalProgress(newFileNames[i], QString(), newFileNames.size(), i);
        emit totalProgress(newFileNames[i], QString(), newFileNames.size(), i + 1);
    }
    emit finished();
}

bool SFtpFileManager::uploadFile(QString const& srcFileName, QString const& dstfileName)
{
    QFile file(srcFileName);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    qint64 filesize = file.size();
    ssh::File::Ptr remotefile = sftpSession->openForWrite(dstfileName.toStdString().c_str(),
                                                          filesize);
    if(!remotefile)
        return false;
    qint64 writedsize = 0;
    while(writedsize < filesize)
    {
        char data[1024];
        qint64 size = file.read(data, sizeof(data));
        if(size <= 0)
            break;
        ssize_t write_size = remotefile->write(data, size);
        if(write_size != size)
            break;
        writedsize += size;
        emit fileProgress(filesize, writedsize);
    }
    if(writedsize != filesize)
        return false;
    return true;
}

bool SFtpFileManager::downloadFile(QString const& srcFileName, QString const& dstfileName)
{
    QFile file(srcFileName);
    if(!file.open(QIODevice::WriteOnly))
        return false;

    ssh::File::Ptr remotefile = sftpSession->openForRead(dstfileName.toStdString().c_str());
    if(!remotefile)
        return false;
    return true;
}
