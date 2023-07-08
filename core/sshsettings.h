#ifndef SSHSETTINGS_H
#define SSHSETTINGS_H
#include <QString>
#include <QList>
#include <QJsonObject>
#include <memory>

struct SSHSettings
{
    using Ptr = std::shared_ptr<SSHSettings>;
    QString name;
    QString hostName;
    QString userName;
    QString passWord;
    QString privateKeyFileName;
    int port = 22;
    bool usePrivateKey = false;
    QString key() const;
};

class SSHSettingsManager
{
public:
    SSHSettingsManager();
    using Ptr = std::shared_ptr<SSHSettingsManager>;

    int size() const { return settingsList_.size(); }
    SSHSettings::Ptr settings(int index) const;

    void addSettings(SSHSettings::Ptr const& settings);
    void removeSettings(SSHSettings::Ptr const& settings);
    void upSettings(int index);
    void downSettings(int index);

    bool save(QString const& fileName);
    bool load(QString const& fileName);
private:
    SSHSettings::Ptr createSettings(QJsonObject const& obj);
private:
    QList<SSHSettings::Ptr> settingsList_;
};

#endif // SSHSETTINGS_H
