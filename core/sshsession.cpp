#include "sshsession.h"

SSHSession::SSHSession(QObject *parent)
    : QObject(parent)
    , sessioin_(new ssh::Session())
{
}

void SSHSession::start(SSHSettings const& settings)
{
    sessioin_->set_host(settings.hostName.toStdString().c_str());
    sessioin_->set_port(settings.port);
    username_ = settings.userName.toStdString();
    sessioin_->set_user(username_.c_str());

    if(!sessioin_->connect())
    {
        emit connectionError(sessioin_->error());
        return;
    }

    if(!sessioin_->verify())
    {
        emit connectionError("verify is failed");
        return;
    }

    if(!sessioin_->login(settings.passWord.toStdString().c_str()))
    {
        emit connectionError("login is failed");
        return;
    }
    emit connected();
}

void SSHSession::stop()
{
    sessioin_->disconnect();
    emit unconnected();
}

