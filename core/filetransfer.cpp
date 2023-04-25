#include "filetransfer.h"
#include "filemanager.h"

FileTransfer::FileTransfer(QObject *parent)
    : QObject(parent)
    , worker_(new FileManager())
{
    qRegisterMetaType<FileNames>("FileNames");
    FileManager* worker = new FileManager();
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &FileTransfer::onCopyFiles, worker, &FileManager::copyFiles);
    connect(this, &FileTransfer::onMoveFiles, worker, &FileManager::moveFiles);
    connect(this, &FileTransfer::onDelereFiles, worker, &FileManager::delereFiles);
    connect(worker, &FileManager::totalProgress, this, &FileTransfer::totalProgress);
    connect(worker, &FileManager::fileProgress, this, &FileTransfer::fileProgress);
    connect(worker, &FileManager::finished, this, &FileTransfer::finished);
    connect(worker, &FileManager::error, this, &FileTransfer::error);

    worker_ = worker;
    workerThread.start();
}

FileTransfer::~FileTransfer()
{
    workerThread.quit();
    workerThread.wait();
}

void FileTransfer::copyFiles(FileNames const& fileNames)
{
    emit onCopyFiles(fileNames);
}

void FileTransfer::moveFiles(FileNames const& fileNames)
{
    emit onMoveFiles(fileNames);
}

void FileTransfer::delereFiles(QStringList const& fileNames)
{
    emit onDelereFiles(fileNames);
}

void FileTransfer::cancel()
{
    worker_->cancel();
}
