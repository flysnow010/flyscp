#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include "filename.h"

#include <QObject>

class FileManager : public QObject
{
    Q_OBJECT
public:
    explicit FileManager(QObject *parent = nullptr);

    static quint32 Progress(void *pData,
                            qint64 TotalFileSize,
                            qint64 TotalBytesTransferred);

public slots:
    void copyFiles(FileNames const& fileNames);
    void moveFiles(FileNames const& fileNames);
    void delereFiles(QStringList const& fileNames);
    void searchFiles(QString const& filePath,
                     QString const& filter);
    void cancel();

signals:
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
    quint32 onProgress(qint64 TotalFileSize,
                       qint64 TotalBytesTransferred);
    void findFiles(QString const& filePath,
                   QString const& filter);

    bool singled() { return signal_; }
    void doSignal() { signal_ = true; };
private:
    volatile bool signal_;
};

#endif // FILEMANAGER_H
