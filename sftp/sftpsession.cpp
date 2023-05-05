#include "sftpsession.h"
#include "ssh/channel.h"

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
        emit connectionError(sessioin->error());
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

std::string SFtpSession::homeDir()
{
    ssh::Channel channel(*sessioin);
    if(!channel.open()
        || !channel.exec("pwd")
        || !channel.poll(1000))
        return std::string();

    char buf[512];
    std::string dir;

    int size = channel.read_nonblocking(buf, sizeof(buf));
    if(size > 0)
        return std::string(buf, size -1);
    return std::string();
}
