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

    ssh::AuthMethod method = sessioin_->auth_method(
                settings.userName.toStdString().c_str());

    if(ssh::isUnknowOfAuthMethod(method))
    {
        emit connectionError("Unknow auth method!");
        return;
    }
    else if(ssh::isPubKeyOfAuthMethod(method) && settings.usePrivateKey)
    {
        if(!sessioin_->login_by_prikey(settings.userName.toStdString().c_str(),
                             settings.privateKeyFileName.toStdString().c_str()))
        {
            emit connectionError("Pubkey's login is failed!");
            return;
        }
    }
    else if(ssh::isPasswordOfAuthMethod(method))
    {
        if(!sessioin_->login(settings.userName.toStdString().c_str(),
                             settings.passWord.toStdString().c_str()))
        {
            emit connectionError("Password's login is failed!");
            return;
        }
    }
    else if(ssh::isInteractiveOfAuthMethod(method))
    {
        if(!sessioin_->login_by_interactive(
                    settings.userName.toStdString().c_str(),
                    settings.passWord.toStdString().c_str()))
        {
            emit connectionError("Interactive's login is failed!");
            return;
        }
    }
    emit connected();
}

void SSHSession::stop()
{
    sessioin_->disconnect();
    emit unconnected();
}

