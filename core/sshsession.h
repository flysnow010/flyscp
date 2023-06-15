#ifndef SSHSESSION_H
#define SSHSESSION_H
#include "sshsettings.h"
#include "ssh/session.h"

#include <QObject>

class SSHSession : public QObject
{
    Q_OBJECT
public:
    explicit SSHSession(QObject *parent = nullptr);

    std::string userName() const { return username_; }
    ssh::Session::Ptr sessioin() const { return sessioin_; }
public slots:
    void start(SSHSettings const& settings);
    void stop();

signals:
    void connected();
    void unconnected();
    void connectionError(QString const& error);
private:
    ssh::Session::Ptr sessioin_;
    std::string username_;
};

#endif // SSHSESSION_H
