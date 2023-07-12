#include "remotefiletransfer.h"
#include "remotefilemanager.h"

RemoteFileTransfer::RemoteFileTransfer(RemoteFileManager* worker, QObject *parent)
    : QObject(parent)
    , worker_(0)
{
    qRegisterMetaType<FileNames>("FileNames");
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);

    connect(this, &RemoteFileTransfer::onUploadFiles, worker, &RemoteFileManager::uploadFiles);
    connect(this, &RemoteFileTransfer::onDownloadFile, worker, &RemoteFileManager::downloadFile);
    connect(this, &RemoteFileTransfer::onDownloadFiles, worker, &RemoteFileManager::downloadFiles);
    connect(this, &RemoteFileTransfer::onSearchFiles, worker, &RemoteFileManager::searchFiles);
    connect(this, &RemoteFileTransfer::onDeleteFiles, worker, &RemoteFileManager::deleteFiles);

    connect(worker, &RemoteFileManager::totalProgress, this, &RemoteFileTransfer::totalProgress);
    connect(worker, &RemoteFileManager::fileProgress, this, &RemoteFileTransfer::fileProgress);
    connect(worker, &RemoteFileManager::currentFolder, this, &RemoteFileTransfer::currentFolder);
    connect(worker, &RemoteFileManager::foundFile, this, &RemoteFileTransfer::foundFile);
    connect(worker, &RemoteFileManager::foundFolder, this, &RemoteFileTransfer::foundFolder);
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

void RemoteFileTransfer::uploadFiles(QStringList const& srcFileNames,
                                     QString const& dstFilePath)
{
    emit onUploadFiles(srcFileNames, dstFilePath);
}

void RemoteFileTransfer::downloadFile(QString const& fileName,
                                      QString const& dstFilePath)
{
    emit onDownloadFile(fileName, dstFilePath);
}

void RemoteFileTransfer::downloadFiles(QStringList const& srcFileNames,
                                       QString const& srcFilePath,
                                       QString const& dstFilePath)
{
    emit onDownloadFiles(srcFileNames, srcFilePath, dstFilePath);
}

void RemoteFileTransfer::searchFiles(QString const& filePath,
                                     QString const& filter)
{
    emit onSearchFiles(filePath, filter);
}

void RemoteFileTransfer::deleteFiles(QStringList const& fileNames,
                                     QString const& filePath,
                                     bool isDst)
{
    emit onDeleteFiles(fileNames, filePath, isDst);
}

void RemoteFileTransfer::cancel()
{
    worker_->cancel();
}
