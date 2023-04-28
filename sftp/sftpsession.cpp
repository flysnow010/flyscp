#include "sftpsession.h"

SFtpSession::SFtpSession(QObject *parent)
    : QObject(parent)
    , sessioin(new ssh::Session())
{
}

void SFtpSession::start(SSHSettings const& settings)
{
    sessioin->setHost(settings.hostName.toStdString().c_str());
    sessioin->setPort(settings.port);
    sessioin->setUser(settings.userName.toStdString().c_str());

    if(!sessioin->connect())
    {
        emit connectionError("connect is timeout");
        return;
    }

    if(!sessioin->verify())
    {
        emit connectionError("verify is failed");
        return;
    }

    if(!sessioin->login(settings.passWord.toStdString().c_str()))
    {
        emit connectionError("login is failed");
        return;
    }
    if(!sftp)
        sftp =ssh::SFtp::Ptr(new ssh::SFtp(*sessioin));
    sftp->init();
    emit connected();
}

void SFtpSession::stop()
{
    sessioin->disconnect();
    emit unconnected();
}
