#include "sftpsession.h"
#include "ssh/channel.h"
#include "ssh/dir.h"
#include <fcntl.h>

SFtpSession::SFtpSession(QObject *parent)
    : QObject(parent)
    , sessioin(new ssh::Session())
{
}

void SFtpSession::start(SSHSettings const& settings)
{
    sessioin->set_host(settings.hostName.toStdString().c_str());
    sessioin->set_port(settings.port);
    username_ = settings.userName.toStdString();
    sessioin->set_user(username_.c_str());

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
        sftp = ssh::SFtp::Ptr(new ssh::SFtp(*sessioin));
    if(!sftp->init())
    {
        sftp.reset();
        scp = ssh::Scp::Ptr(new ssh::Scp(*sessioin));
    }
    emit connected();
}

void SFtpSession::stop()
{
    sessioin->disconnect();
    emit unconnected();
}

ssh::File::Ptr SFtpSession::openForRead(const char* filename)
{
    ssh::File::Ptr file = createFile();
    if(file->open(filename, O_RDONLY, 0644))
        return file;
    return ssh::File::Ptr();
}

ssh::File::Ptr SFtpSession::openForWrite(const char* filename, uint64_t filesize)
{
    ssh::File::Ptr file = createFile();
    file->set_filesize(filesize);
    if(file->open(filename, O_CREAT | O_WRONLY, 0644))
        return file;
    return ssh::File::Ptr();
}

ssh::File::Ptr SFtpSession::createFile()
{
    if(sftp)
        return ssh::File::Ptr(new ssh::File(*sftp));
    else
        return ssh::File::Ptr(new ssh::File(*sessioin));
}

bool SFtpSession::createDir(std::string const& path)
{
    if(!scp)
        return false;

    std::string dirname = ssh::Dir::dirname(path.c_str());
    std::string basename = ssh::Dir::basename(path.c_str());
    if(scp->open(dirname.c_str(), false))
    {
        scp->mkdir(basename.c_str());
        scp->close();
        return true;
    }
    return false;
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
