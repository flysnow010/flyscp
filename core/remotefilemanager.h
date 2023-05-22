#ifndef REMOTEFILEMANAGER_H
#define REMOTEFILEMANAGER_H
#include "filename.h"

#include <QObject>

class RemoteFileManager : public QObject
{
    Q_OBJECT
public:
    explicit RemoteFileManager(QObject *parent = nullptr);
    virtual ~RemoteFileManager(){};

public slots:
    void uploadFiles(FileNames const& fileNames);
    void downloadFiles(FileNames const& fileNames);
    void delereFiles(QStringList const& remoteFileNames, bool isDst);
    void cancel();

signals:
    void totalProgress(QString const& srcFilename, QString const& dstFilename,
                       int totalSize, int totalSizeTransferred);
    void fileProgress(qint64 totalFileSize, qint64 totalBytesTransferred);
    void error(QString const& e);
    void finished();
protected:
    virtual void doUploadFiles(FileNames const& fileNames);
    virtual void doDownloadFiles(FileNames const& fileNames);
    virtual void doDelereFiles(QStringList const& remoteFileNames, bool isDst) = 0;
protected:
    bool singled() { return signal_; }
    void doSignal() { signal_ = true; };
private:
    volatile bool signal_;

};

#endif // REMOTEFILEMANAGER_H
