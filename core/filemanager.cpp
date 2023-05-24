#include "filemanager.h"
#include <windows.h>

#include <QDebug>
#include <QFileInfo>
#include <QDir>

static DWORD ProgressCallback(
    LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER /*StreamSize*/,
    LARGE_INTEGER /*StreamBytesTransferred*/,
    DWORD /*dwStreamNumber*/,
    DWORD /*dwCallbackReason*/,
    HANDLE /*hSourceFile*/,
    HANDLE /*hDestinationFile*/,
    LPVOID lpData
)
{
    return FileManager::Progress(lpData, TotalFileSize.QuadPart, TotalBytesTransferred.QuadPart);
}

FileManager::FileManager(QObject *parent)
    : QObject(parent)
    , signal_(false)
{

}

quint32 FileManager::Progress(void *pData, qint64 TotalFileSize, qint64 TotalBytesTransferred)
{
    FileManager* fileManger = (FileManager *)pData;
    return fileManger->onProgress(TotalFileSize, TotalBytesTransferred);
}

quint32 FileManager::onProgress(qint64 TotalFileSize, qint64 TotalBytesTransferred)
{
    emit fileProgress(TotalFileSize, TotalBytesTransferred);
    if(singled())
      return  PROGRESS_CANCEL;
    return PROGRESS_CONTINUE;
}

void FileManager::delereFiles(QStringList const& fileNames)
{
    QDir dir;
    QStringList newFileNames;
    FileNames::MakeFileNames(fileNames, newFileNames);
    for(int i = 0; i < newFileNames.size(); i++)
    {
        if(singled())
            break;

        emit totalProgress(newFileNames[i], QString(), newFileNames.size(), i);
        QFileInfo fileInfo(newFileNames[i]);
        if(fileInfo.isDir())
            dir.rmdir(newFileNames[i]);
        else
           dir.remove(newFileNames[i]);
        emit totalProgress(newFileNames[i], QString(), newFileNames.size(), i + 1);
    }
    emit finished();
}

void FileManager::copyFiles(FileNames const& fileNames)
{
    FileNames newFileNames;
    FileNames::MakeFileNames(fileNames, newFileNames);
    for(int i = 0; i < newFileNames.size(); i++)
    {
        if(singled())
            break;
        emit totalProgress(newFileNames[i].src, newFileNames[i].dst, newFileNames.size(), i);
        QDir().mkpath(QFileInfo(newFileNames[i].dst).path());
        CopyFileEx(newFileNames[i].src.toStdWString().c_str(),
                   newFileNames[i].dst.toStdWString().c_str(),
                   ProgressCallback, this, 0,   COPY_FILE_OPEN_SOURCE_FOR_WRITE);
        emit totalProgress(newFileNames[i].src, newFileNames[i].dst, newFileNames.size(), i + 1);
    }
    emit finished();
}

void FileManager::moveFiles(FileNames const& fileNames)
{
    FileNames newFileNames;
    FileNames::MakeFileNames(fileNames, newFileNames);
    for(int i = 0; i < newFileNames.size(); i++)
    {
        if(singled())
            break;
        emit totalProgress(newFileNames[i].src, newFileNames[i].dst, newFileNames.size(), i);
        QDir().mkpath(QFileInfo(newFileNames[i].dst).path());
        MoveFileWithProgress(newFileNames[i].src.toStdWString().c_str(),
                             newFileNames[i].dst.toStdWString().c_str(),
                             ProgressCallback, this,
                             MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
        emit totalProgress(newFileNames[i].src, newFileNames[i].dst, newFileNames.size(), i + 1);
    }
    foreach(auto const& fileName, fileNames)
    {
        if(QFileInfo(fileName.src).isDir())
            QDir(fileName.src).removeRecursively();
    }
    emit finished();
}

void FileManager::cancel()
{
    doSignal();
}
