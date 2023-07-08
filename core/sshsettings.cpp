#include "sshsettings.h"
#include <QJsonArray>
#include <QJsonDocument>


QString SSHSettings::key() const
{
    return QString("%1@%2:%3").arg(userName, hostName).arg(port);
}

SSHSettingsManager::SSHSettingsManager()
{}

SSHSettings::Ptr SSHSettingsManager::settings(int index) const
{
    if(index >=0 && index < settingsList_.size())
        return settingsList_.at(index);
    return SSHSettings::Ptr();
}

void SSHSettingsManager::addSettings(SSHSettings::Ptr const& settings)
{
        settingsList_.append(settings);
}

void SSHSettingsManager::removeSettings(SSHSettings::Ptr const& settings)
{
    settingsList_.removeAll(settings);
}

void SSHSettingsManager::upSettings(int index)
{
    int newIndex = index - 1;
    if(newIndex >= 0 && index < settingsList_.size())
    {
        SSHSettings::Ptr settings = settingsList_.at(newIndex);
        settingsList_[newIndex] = settingsList_.at(index);
        settingsList_[index] = settings;
    }
}

void SSHSettingsManager::downSettings(int index)
{
    int newIndex = index + 1;
    if(newIndex > 0 && newIndex < settingsList_.size())
    {
        SSHSettings::Ptr settings = settingsList_.at(newIndex);
        settingsList_[newIndex] = settingsList_.at(index);
        settingsList_[index] = settings;
    }
}

bool SSHSettingsManager::save(QString const& fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly))
        return false;

    QJsonArray settingsList;
    for(int i = 0; i < settingsList_.size(); i++)
    {
        QJsonObject settings;
        settings.insert("name", settingsList_[i]->name);
        settings.insert("hostName", settingsList_[i]->hostName);
        settings.insert("userName", settingsList_[i]->userName);
        settings.insert("privateKeyFileName", settingsList_[i]->privateKeyFileName);
        settings.insert("usePrivateKey", settingsList_[i]->usePrivateKey);
        settings.insert("port", settingsList_[i]->port);
        settingsList.append(settings);
    }

    QJsonObject settingsManager;
    settingsManager.insert("version", "1.0");
    settingsManager.insert("settingsList", settingsList);

    QJsonDocument doc;
    doc.setObject(settingsManager);
    file.write(doc.toJson());
    return true;
}

bool SSHSettingsManager::load(QString const& fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError json_error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &json_error);
    if(json_error.error != QJsonParseError::NoError)
        return false;

    QJsonObject settingsManager = doc.object();
    QJsonArray settingsList = settingsManager.value("settingsList").toArray();
    for(int i = 0; i < settingsList.size(); i++)
    {
        QJsonObject object = settingsList.at(i).toObject();
        SSHSettings::Ptr settings = createSettings(object);
        if(settings)
            settingsList_ << settings;
    }

    return true;
}

SSHSettings::Ptr SSHSettingsManager::createSettings(QJsonObject const& obj)
{
    SSHSettings::Ptr settings = SSHSettings::Ptr(new SSHSettings());

    settings->name = obj.value("name").toString();
    settings->hostName = obj.value("hostName").toString();
    settings->userName = obj.value("userName").toString();
    settings->privateKeyFileName = obj.value("privateKeyFileName").toString();
    settings->usePrivateKey = obj.value("usePrivateKey").toBool();
    settings->port = obj.value("port").toInt();

    return settings;
}
