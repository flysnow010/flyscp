#ifndef SFTPSESSION_H
#define SFTPSESSION_H
#include "core/sshsettings.h"
#include "ssh/sftp.h"
#include "ssh/scp.h"
#include "ssh/dir.h"
#include "ssh/file.h"
#include "ssh/session.h"

#include <QObject>

class SFtpSession : public QObject
{
    Q_OBJECT
public:
    explicit SFtpSession(QObject *parent = nullptr);

    inline ssh::DirPtr home() const
    {
        return sftp ? sftp->home() : scp->home();
    }
    inline ssh::DirPtr dir(std::string const& path) const
    {
        return sftp ? sftp->dir(path.c_str()) : scp->dir(path.c_str());
    }

    ssh::File::Ptr openForRead(const char* filename);
    ssh::File::Ptr openForWrite(const char* filename, uint64_t filesize = 0);

    std::string homeDir();
    std::string userName() const { return username_; }
    std::string error() const;

    bool isSftp() const { return sftp != 0; }
public slots:
    void start(SSHSettings const& settings);
    void stop();

signals:
    void connected();
    void unconnected();
    void connectionError(QString const& error);
private:
    ssh::File::Ptr createFile();

private:
    ssh::Session::Ptr sessioin;
    ssh::SFtp::Ptr sftp;
    ssh::Scp::Ptr scp;
    std::string username_;
    std::string homedir_;
};

#endif // SFTPSESSION_H
