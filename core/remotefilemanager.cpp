#include "remotefilemanager.h"

RemoteFileManager::RemoteFileManager(QObject *parent)
    : QObject(parent)
    , signal_(false)
{

}

void RemoteFileManager::uploadFiles(FileNames const& fileNames)
{
    doUploadFiles(fileNames);
}
void RemoteFileManager::downloadFiles(FileNames const& fileNames)
{
    doDownloadFiles(fileNames);
}

void RemoteFileManager::delereFiles(QStringList const& remoteFileNames, bool isDst)
{
    doDelereFiles(remoteFileNames, isDst);
}

void RemoteFileManager::cancel()
{
    doSignal();
}

void RemoteFileManager::doUploadFiles(FileNames const& fileNames)
{
    Q_UNUSED(fileNames)
}

void RemoteFileManager::doDownloadFiles(FileNames const& fileNames)
{
    Q_UNUSED(fileNames)
}