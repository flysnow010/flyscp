#include "sftpfilemanager.h"
#include "sftp/sftpsession.h"
#include "ssh/fileinfo.h"
#include "filename.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>

SFtpFileManager::SFtpFileManager(SFtpSession* sftp, QObject *parent)
    : RemoteFileManager(parent)
    , sftpSession(sftp)
{
}

void SFtpFileManager::uploadFiles(QStringList const& srcFileNames,
                                  QString const& dstFilePath)
{
    FileNames newFileNames;
    FileNames fileNames = FileNames::GetFileNames(srcFileNames, dstFilePath);

    FileNames::MakeFileNames(fileNames, newFileNames);
    mkdirs(newFileNames, dstFilePath);
    for(int i = 0; i < newFileNames.size(); i++)
    {
        if(singled())
            break;

        emit totalProgress(newFileNames[i].src,
                           newFileNames[i].dst,
                           newFileNames.size(),
                           i);
        uploadFile(newFileNames[i].src, newFileNames[i].dst);
        emit totalProgress(newFileNames[i].src,
                           newFileNames[i].dst,
                           newFileNames.size(),
                           i + 1);
    }
    emit finished();
}

void SFtpFileManager::downloadFile(QString const& fileName,
                  QString const& dstFilePath)
{
    if(!getFileSize(fileName))
    {
        emit finished();
        return;
    }

    QString dstFileName = QDir(dstFilePath)
            .filePath(QFileInfo(fileName).fileName());

    emit totalProgress(fileName, dstFileName, 1, 0);
    downloadOneFile(fileName, dstFileName);
    emit totalProgress(fileName, dstFileName, 1, 1);
    emit finished();
}

void SFtpFileManager::downloadFiles(QStringList const& srcFileNames,
                                    QString const& srcFilePath,
                                    QString const& dstFilePath)
{
    FileNames newFileNames = getFileNames(srcFileNames,
                                          srcFilePath,
                                          dstFilePath);

    for(int i = 0; i < newFileNames.size(); i++)
    {
        if(singled())
            break;

        emit totalProgress(newFileNames[i].src,
                           newFileNames[i].dst,
                           newFileNames.size(),
                           i);
        QDir().mkpath(QFileInfo(newFileNames[i].dst).path());
        downloadOneFile(newFileNames[i].src, newFileNames[i].dst);
        emit totalProgress(newFileNames[i].src,
                           newFileNames[i].dst,
                           newFileNames.size(),
                           i + 1);
    }
    emit finished();
}

void SFtpFileManager::searchFiles(QString const& filePath,
                             QString const& filter)
{
    findFiles(filePath, filter);
    emit finished();
}

void SFtpFileManager::deleteFiles(QStringList const& fileNames,
                                  QString const& filePath,
                                  bool isDst)
{
    Q_UNUSED(isDst)
    FileInfos newFileInfos = getFileNames(fileNames, filePath);
    ssh::DirPtr dir = sftpSession->dir(filePath.toStdString());
    for(int i = 0; i < newFileInfos.size(); i++)
    {
        if(singled())
            break;

        emit totalProgress(newFileInfos[i].fileName,
                           QString(),
                           newFileInfos.size(),
                           i);
        std::string filename = newFileInfos[i].fileName.toStdString();
        if(newFileInfos[i].isDir)
            dir->rmdir(filename.c_str());
        else
            dir->rmfile(filename.c_str());
        emit totalProgress(newFileInfos[i].fileName,
                           QString(),
                           newFileInfos.size(),
                           i + 1);
    }
    emit finished();
}

bool SFtpFileManager::uploadFile(QString const& srcFileName,
                                 QString const& dstfileName)
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
    while(writedsize < filesize && !singled())
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

bool SFtpFileManager::downloadOneFile(QString const& srcFileName,
                                   QString const& dstfileName)
{
    ssh::File::Ptr remotefile = sftpSession->openForRead(srcFileName.toStdString().c_str());
    if(!remotefile)
        return false;

    QFile file(dstfileName);
    if(!file.open(QIODevice::WriteOnly))
        return false;

    qint64 filesize = fileSizes[srcFileName];
    qint64 writedsize = 0;
    while(writedsize < filesize && !singled())
    {
        char data[1024];
        ssize_t size = remotefile->read(data, sizeof(data));
        if(size <= 0)
            break;
        file.write(data, size);
        writedsize += size;
        emit fileProgress(filesize, writedsize);
    }
    if(writedsize != filesize)
        return false;
    return true;
}

FileNames SFtpFileManager::getFileNames(QStringList const& srcFileNames,
                                        QString const& srcFilePath,
                                        QString const& dstFilePath)
{
    ssh::DirPtr dirPtr = sftpSession->dir(srcFilePath.toStdString());
    if(!dirPtr)
        return FileNames();
    fileSizes.clear();
    FileNames newFileNames;
    QDir srcDir(srcFilePath);
    QDir dstDir(dstFilePath);
    ssh::FileInfos fileInfos = dirPtr->fileinfos();

    foreach(auto fileName, srcFileNames)
    {
        ssh::FileInfoPtr fileInfo = fileInfos.find(fileName.toStdString());
        if(!fileInfo)
            continue;
        QString childFilePath = srcDir.filePath(fileName);
        if(fileInfo->is_dir())
            newFileNames << getFileNames(childFilePath, dstDir.filePath(fileName));
        else
        {
            FileName filename;
            filename.src = childFilePath;
            filename.dst = dstDir.filePath(fileName);
            newFileNames << filename;
            fileSizes[childFilePath] = fileInfo->size();
        }
    }
    return newFileNames;
}

void SFtpFileManager::findFiles(QString const& filePath,
               QString const& filter)
{
    ssh::DirPtr dirPtr = sftpSession->dir(filePath.toStdString());
    if(!dirPtr || singled())
        return;

    QDir dir(filePath);
    ssh::FileInfos fileInfos = dirPtr->fileinfos();
    QRegExp regExp(filter, Qt::CaseSensitive, QRegExp::Wildcard);
    bool isMatch = filter == "*" ? true : false;
    emit currentFolder(filePath);
    for(auto fileInfo: fileInfos)
    {
        if(singled())
            break;
        QString name = QString::fromStdString(fileInfo->name());
        QString childFilePath = dir.filePath(name);
        if(isMatch || regExp.exactMatch(childFilePath))
        {
            if(fileInfo->is_dir())
                emit foundFolder(childFilePath);
            else
                emit foundFile(childFilePath);
        }
        if(fileInfo->is_dir())
            findFiles(childFilePath, filter);
    }
}

FileNames SFtpFileManager::getFileNames(QString const& srcFilePath,
                                        QString const& dstFilePath)
{
    ssh::DirPtr dirPtr = sftpSession->dir(srcFilePath.toStdString());
    if(!dirPtr)
        return FileNames();

    FileNames newFileNames;
    QDir srcDir(srcFilePath);
    QDir dstDir(dstFilePath);
    ssh::FileInfos fileInfos = dirPtr->fileinfos();

    for(auto const& fileInfo: fileInfos)
    {
        QString name = QString::fromStdString(fileInfo->name());
        QString childFilePath = srcDir.filePath(name);
        if(fileInfo->is_dir())
            newFileNames << getFileNames(childFilePath, dstDir.filePath(name));
        else
        {
            FileName filename;
            filename.src = childFilePath;
            filename.dst = dstDir.filePath(name);
            newFileNames << filename;
            fileSizes[childFilePath] = fileInfo->size();
        }
    }

    return newFileNames;
}

FileInfos SFtpFileManager::getFileNames(QStringList const& fileNames,
                                        QString const& filePath)
{
    ssh::DirPtr dirPtr = sftpSession->dir(filePath.toStdString());
    if(!dirPtr)
        return FileInfos();

    FileInfos newFileInfos;
    ssh::FileInfos fileInfos = dirPtr->fileinfos();
    QDir dir(filePath);

    foreach(auto fileName, fileNames)
    {
        ssh::FileInfoPtr fileInfo = fileInfos.find(fileName.toStdString());
        if(!fileInfo)
            continue;
        QString childFilePath = dir.filePath(fileName);
        if(!fileInfo->is_dir())
            newFileInfos << FileInfo(childFilePath, false);
        else
        {
            newFileInfos << getFileNames(childFilePath);
            newFileInfos << FileInfo(childFilePath, true);
        }
    }

    return newFileInfos;
}

FileInfos SFtpFileManager::getFileNames(QString const& filePath)
{
    ssh::DirPtr dir = sftpSession->dir(filePath.toStdString());
    if(!dir)
        return FileInfos();

    FileInfos newFileInfos;
    QDir srcDir(filePath);
    ssh::FileInfos fileInfos = dir->fileinfos();

    for(auto const& fileInfo: fileInfos)
    {
        QString childFilePath = srcDir.filePath(QString::fromStdString(fileInfo->name()));
        if(!fileInfo->is_dir())
            newFileInfos << FileInfo(childFilePath, false);
        else
        {
            newFileInfos << getFileNames(childFilePath);
            newFileInfos << FileInfo(childFilePath, true);
        }
    }

    return newFileInfos;
}

bool SFtpFileManager::getFileSize(QString const& fileName)
{
    QFileInfo fileInfo(fileName);
    ssh::DirPtr dir = sftpSession->dir(fileInfo.path().toStdString());
    if(!dir)
        return false;

    ssh::FileInfos fileInfos = dir->fileinfos();
    ssh::FileInfoPtr fileInfoPtr = fileInfos.find(fileInfo.fileName().toStdString());
    if(!fileInfoPtr)
        return false;

    fileSizes.clear();
    fileSizes[fileName] = fileInfoPtr->size();
    return true;
}

void SFtpFileManager::mkdirs(FileNames const& fileNames, QString const& dstFilePath)
{
    if(fileNames.isEmpty())
        return;

    QStringList dirnames;
    foreach(auto const& fileName, fileNames)
    {
        QString path  = QFileInfo(fileName.dst).path();
        if(!dirnames.contains(path))
            dirnames << path;
    }
    dirnames.sort();

    QStringList newdirnames;
    foreach(auto const& dirname, dirnames)
    {
        QString name = dirname;
        while(name != dstFilePath)
        {
            if(!newdirnames.contains(name))
                newdirnames << name;
            name = QFileInfo(name).path();
        }
    }
    newdirnames.sort();

    foreach(auto const& dirname, newdirnames)
        sftpSession->mkdir(dirname.toStdString());
}
