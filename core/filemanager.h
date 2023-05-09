#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include "filename.h"

#include <QObject>

class QFileInfo;
class FileManager : public QObject
{
    Q_OBJECT
public:
    explicit FileManager(QObject *parent = nullptr);

    static quint32 Progress(void *pData, qint64 TotalFileSize, qint64 TotalBytesTransferred);

    static void Property(QString const& fileName);
    static void OpenWith(QString const& fileName);
    static void Execute(QString const& fileName);
    static void Open(QString const& fileName);
public slots:
    void copyFiles(FileNames const& fileNames);
    void moveFiles(FileNames const& fileNames);
    void delereFiles(QStringList const& fileNames);
    void cancel();

signals:
    void totalProgress(QString const& srcFilename, QString const& dstFilename,
                       int totalSize, int totalSizeTransferred);
    void fileProgress(qint64 totalFileSize, qint64 totalBytesTransferred);
    void error(QString const& e);
    void finished();
private:
    quint32 onProgress(qint64 TotalFileSize, qint64 TotalBytesTransferred);
    void makeFileNames(FileNames const& fileNames, FileNames & newFileNames);
    void makeFileNames(QStringList const& fileNames, QStringList & newFileNames);
    void findFilenames(FileName const& fileName, FileNames &fileNames);
    void findFilenames(QString const& fileName, QStringList &fileNames);

    bool singled() { return signal_; }
    void doSignal() { signal_ = true; };
private:
    volatile bool signal_;
};

#endif // FILEMANAGER_H
