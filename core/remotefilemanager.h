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
    virtual void uploadFiles(QStringList const& srcFileNames,
                             QString const& dstFilePath) = 0;
    virtual void downloadFiles(QStringList const& srcFileNames,
                               QString const& srcFilePath,
                               QString const& dstFilePath) = 0;
    virtual void deleteFiles(QStringList const& fileNames,
                             QString const& filePath,
                             bool isDst) = 0;
    void cancel();

signals:
    void totalProgress(QString const& srcFilename,
                       QString const& dstFilename,
                       int totalSize,
                       int totalSizeTransferred);
    void fileProgress(qint64 totalFileSize,
                      qint64 totalBytesTransferred);
    void error(QString const& e);
    void finished();

protected:
    bool singled() { return signal_; }
    void doSignal() { signal_ = true; };
private:
    volatile bool signal_;

};

#endif // REMOTEFILEMANAGER_H
