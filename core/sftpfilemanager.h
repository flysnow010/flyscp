#ifndef SFTPFILEMANAGER_H
#define SFTPFILEMANAGER_H

#include "remotefilemanager.h"

class SFtpSession;
class SFtpFileManager : public RemoteFileManager
{
public:
    explicit SFtpFileManager(SFtpSession* sftp, QObject *parent = nullptr);
    ~SFtpFileManager(){}

public slots:
    void uploadFiles(QStringList const& srcFileNames,
                     QString const& dstFilePath) override;
    void downloadFile(QString const& fileName,
                      QString const& dstFilePath) override;
    void downloadFiles(QStringList const& srcFileNames,
                       QString const& srcFilePath,
                       QString const& dstFilePath) override;
    void searchFiles(QString const& filePath,
                                 QString const& filter) override;
    void deleteFiles(QStringList const& fileNames,
                     QString const& filePath,
                     bool isDst) override;
private:
    bool uploadFile(QString const& srcFileName,
                    QString const& dstfileName);
    bool downloadOneFile(QString const& srcFileName,
                      QString const& dstfileName);
    void findFiles(QString const& filePath,
                   QString const& filter);
    FileNames getFileNames(QStringList const& srcFileNames,
                           QString const& srcFilePath,
                           QString const& dstFilePath);
    FileNames getFileNames(QString const& srcFilePath,
                           QString const& dstFilePath);
    FileInfos getFileNames(QStringList const& fileNames,
                           QString const& filePath);
    FileInfos getFileNames(QString const& filePath);
    bool getFileSize(QString const& fileName);
    void mkdirs(FileNames const& fileNames,
                QString const& dstFilePath);
private:
    SFtpSession* sftpSession;
    QMap<QString, qint64> fileSizes;
};

#endif // SFTPFILEMANAGER_H
