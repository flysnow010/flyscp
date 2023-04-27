#ifndef SSHSETTINGS_H
#define SSHSETTINGS_H
#include <QString>

struct SSHSettings
{
    QString hostName;
    QString userName;
    QString privateKeyFileName;
    int port = 22;
    bool usePrivateKey = false;

    QString name() const { return QString("%1 (%2)").arg(hostName).arg(userName); }
};

#endif // SSHSETTINGS_H
