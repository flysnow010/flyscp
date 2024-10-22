#ifndef REMOTEFILETRANSFER_H
#define REMOTEFILETRANSFER_H
#include "filename.h"

#include <QObject>
#include <QThread>

class RemoteFileManager;
class RemoteFileTransfer : public QObject
{
    Q_OBJECT
public:
    explicit RemoteFileTransfer(RemoteFileManager* worker,
                                QObject *parent = nullptr);
    ~RemoteFileTransfer();

public:
    void uploadFiles(QStringList const& srcFileNames,
                     QString const& dstFilePath);
    void downloadFile(QString const& fileName,
                      QString const& dstFilePath);
    void downloadFiles(QStringList const& srcFileNames,
                       QString const& srcFilePath,
                       QString const& dstFilePath);
    void searchFiles(QString const& filePath,
                     QString const& filter);
    void deleteFiles(QStringList const& fileNames,
                     QString const& filePath,
                     bool isDst);
    void cancel();

signals:
    void onUploadFiles(QStringList const& srcFileNames,
                       QString const& dstFilePath);
    void onDownloadFile(QString const& fileName,
                        QString const& dstFilePath);
    void onDownloadFiles(QStringList const& srcFileNames,
                         QString const& srcFilePath,
                         QString const& dstFilePath);
    void onSearchFiles(QString const& filePath,
                       QString const& filter);
    void onDeleteFiles(QStringList const& fileNames,
                       QString const& filePath,
                       bool isDst);

    void totalProgress(QString const& srcFilename,
                       QString const& dstFilename,
                       int totalSize,
                       int totalSizeTransferred);
    void fileProgress(qint64 totalFileSize,
                      qint64 totalBytesTransferred);
    void currentFolder(QString const& filePath);
    void foundFile(QString const& fileName);
    void foundFolder(QString const& filePath);
    void error(QString const& e);
    void finished();
private:
   QThread workerThread;
   RemoteFileManager* worker_;
};

#endif // REMOTEFILETRANSFER_H
