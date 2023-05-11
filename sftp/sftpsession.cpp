#include "sftpsession.h"
#include "ssh/channel.h"
#include <fcntl.h>

SFtpSession::SFtpSession(QObject *parent)
    : QObject(parent)
    , sessioin(new ssh::Session())
{
}

void SFtpSession::start(SSHSettings const& settings)
{
    sessioin->setHost(settings.hostName.toStdString().c_str());
    sessioin->setPort(settings.port);
    username_ = settings.userName.toStdString();
    sessioin->setUser(username_.c_str());

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

ssh::File::Ptr SFtpSession::openForRead(const char* filename)
{
    ssh::File::Ptr file(new ssh::File(*sftp));
    if(file->open(filename, O_RDONLY, 0644))
        return file;
    return ssh::File::Ptr();
}

ssh::File::Ptr SFtpSession::openForWrite(const char* filename)
{
    ssh::File::Ptr file(new ssh::File(*sftp));
    if(file->open(filename, O_CREAT | O_WRONLY, 0644))
        return file;
    return ssh::File::Ptr();
}

std::string SFtpSession::homeDir()
{
    if(!homedir_.empty())
        return homedir_;
    ssh::Channel channel(*sessioin);
    if(!channel.open()
        || !channel.exec("pwd")
        || !channel.poll(1000))
        return homedir_;

    char buf[512];

    int size = channel.read_nonblocking(buf, sizeof(buf));
    if(size > 0)
        homedir_ = std::string(buf, size -1);

    return homedir_;
}

std::string SFtpSession::error() const
{
    return sftp->error();
}
