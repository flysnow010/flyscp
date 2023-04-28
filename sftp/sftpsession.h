#ifndef SFTPSESSION_H
#define SFTPSESSION_H
#include "core/sshsettings.h"
#include "ssh/sftp.h"
#include "ssh/dir.h"
#include "ssh/session.h"

#include <QObject>

class SFtpSession : public QObject
{
    Q_OBJECT
public:
    explicit SFtpSession(QObject *parent = nullptr);

    inline ssh::DirPtr home() const { return sftp->home(); }
    inline ssh::DirPtr dir(QString const& path) const { return sftp->dir(path.toStdString().c_str()); }
public slots:
    void start(SSHSettings const& settings);
    void stop();

signals:
    void connected();
    void unconnected();
    void connectionError(QString const& error);

private:
    ssh::Session::Ptr sessioin;
    ssh::SFtp::Ptr sftp;
};

#endif // SFTPSESSION_H
