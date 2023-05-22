#include "remotefiletransfer.h"
#include "remotefilemanager.h"

RemoteFileTransfer::RemoteFileTransfer(RemoteFileManager* worker, QObject *parent) : QObject(parent)
{
    qRegisterMetaType<FileNames>("FileNames");
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);

    connect(this, &RemoteFileTransfer::onUploadFiles, worker, &RemoteFileManager::uploadFiles);
    connect(this, &RemoteFileTransfer::onDownloadFiles, worker, &RemoteFileManager::downloadFiles);
    connect(this, &RemoteFileTransfer::onDelereFiles, worker, &RemoteFileManager::delereFiles);

    connect(worker, &RemoteFileManager::totalProgress, this, &RemoteFileTransfer::totalProgress);
    connect(worker, &RemoteFileManager::fileProgress, this, &RemoteFileTransfer::fileProgress);
    connect(worker, &RemoteFileManager::finished, this, &RemoteFileTransfer::finished);
    connect(worker, &RemoteFileManager::error, this, &RemoteFileTransfer::error);
    worker_ = worker;
    workerThread.start();
}

RemoteFileTransfer::~RemoteFileTransfer()
{
    workerThread.quit();
    workerThread.wait();
}

void RemoteFileTransfer::uploadFiles(FileNames const& fileNames)
{
    emit onUploadFiles(fileNames);
}

void RemoteFileTransfer::downloadFiles(FileNames const& fileNames)
{
    emit onDownloadFiles(fileNames);
}

void RemoteFileTransfer::delereFiles(QStringList const& remoteFileNames, bool isDst)
{
    emit onDelereFiles(remoteFileNames, isDst);
}

void RemoteFileTransfer::cancel()
{
    worker_->cancel();
}
