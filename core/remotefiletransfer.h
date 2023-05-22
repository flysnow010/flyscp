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
    explicit RemoteFileTransfer(RemoteFileManager* worker, QObject *parent = nullptr);
    ~RemoteFileTransfer();

public:
    void uploadFiles(FileNames const& fileNames);
    void downloadFiles(FileNames const& fileNames);
    void delereFiles(QStringList const& fileNames, QString const& filePath, bool isDst);
    void cancel();
signals:
    void onUploadFiles(FileNames const& fileNames);
    void onDownloadFiles(FileNames const& fileNames);
    void onDelereFiles(QStringList const& fileNames, QString const& filePath, bool isDst);

    void totalProgress(QString const& srcFilename, QString const& dstFilename,
                       int totalSize, int totalSizeTransferred);
    void fileProgress(qint64 totalFileSize, qint64 totalBytesTransferred);
    void error(QString const& e);
    void finished();
private:
   QThread workerThread;
   RemoteFileManager* worker_;
};

#endif // REMOTEFILETRANSFER_H
