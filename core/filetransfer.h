#ifndef FILETRANSFER_H
#define FILETRANSFER_H
#include "filename.h"

#include <QObject>
#include <QThread>

class FileManager;
class FileTransfer : public QObject
{
    Q_OBJECT
public:
    explicit FileTransfer(QObject *parent = nullptr);
    ~FileTransfer();

public:
    void copyFiles(FileNames const& fileNames);
    void moveFiles(FileNames const& fileNames);
    void delereFiles(QStringList const& fileNames);
    void searchFiles(QString const& filePath, QString const& filter);
    void cancel();

signals:
    void onCopyFiles(FileNames const& fileNames);
    void onMoveFiles(FileNames const& fileNames);
    void onDelereFiles(QStringList const& fileNames);
    void onSearchFiles(QString const& filePath, QString const& filter);

    void totalProgress(QString const& srcFilename, QString const& dstFilename,
                       int totalSize, int totalSizeTransferred);
    void fileProgress(qint64 totalFileSize, qint64 totalBytesTransferred);
    void currentFolder(QString const& filePath);
    void foundFile(QString const& fileName);
    void foundFolder(QString const& filePath);
    void finished();
    void error(QString const& e);
private:
   QThread workerThread;
   FileManager* worker_;
};

#endif // FILETRANSFER_H
