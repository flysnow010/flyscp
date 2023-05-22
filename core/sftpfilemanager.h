#ifndef SFTPFILEMANAGER_H
#define SFTPFILEMANAGER_H

#include "remotefilemanager.h"

class SFtpSession;
class SFtpFileManager : public RemoteFileManager
{
public:
    explicit SFtpFileManager(SFtpSession* sftp, QObject *parent = nullptr);
    ~SFtpFileManager(){}
protected:
    void doUploadFiles(FileNames const& fileNames) override;
    void doDownloadFiles(FileNames const& fileNames) override;
    void doDelereFiles(QStringList const& remoteFileNames, bool isDst) override;
private:
    bool uploadFile(QString const& srcFileName, QString const& dstfileName);
    bool downloadFile(QString const& srcFileName, QString const& dstfileName);
private:
    SFtpSession* sftpSession;
};

#endif // SFTPFILEMANAGER_H
