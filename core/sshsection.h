#ifndef SSHSECTION_H
#define SSHSECTION_H
#include "sshsettings.h"
#include "ssh/session.h"

#include <QObject>

class SSHSection : public QObject
{
    Q_OBJECT
public:
    explicit SSHSection(QObject *parent = nullptr);

public slots:
    void start(SSHSettings const& settings);
    void stop();

signals:
    void connected();
    void unconnected();
    void connectionError(QString const& error);
private:
    ssh::Session::Ptr sessioin;
    std::string username_;
};

#endif // SSHSECTION_H
