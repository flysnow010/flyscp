#include "sshsection.h"

SSHSection::SSHSection(QObject *parent)
    : QObject(parent)
    , sessioin(new ssh::Session())
{

}

void SSHSection::start(SSHSettings const& settings)
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
    emit connected();
}

void SSHSection::stop()
{
    sessioin->disconnect();
    emit unconnected();
}

