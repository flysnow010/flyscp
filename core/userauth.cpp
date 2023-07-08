#include "userauth.h"
#include "util/utils.h"
#include "rsa/rsa.h"
#include "pubkey.h"
#include "prikey.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>

namespace  {

QString encryptText(QString const& text)
{
    QString key = QString::fromUtf8((const char *)pubkey, sizeof(pubkey));
    return Rsa::Encrypt(key, text);
}

QString decryptText(QString const& text)
{
    QString key = QString::fromUtf8((const char *)prikey, sizeof(prikey));
    return Rsa::Decrypt(key, text);
}

}
QString UserAuth::hash(QString const& text)
{
    return QString::fromUtf8(QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Md5).toBase64());
}


UserAuthManager::UserAuthManager(QObject *parnet)
    : QObject(parnet)
{
}

UserAuth::Ptr UserAuthManager::findUserAuth(QString const& key)
{
    for(int i = 0; i < userAuths_.size(); i++)
    {
        if(userAuths_[i]->key == key)
            return userAuths_[i];
    }
    return UserAuth::Ptr();
}

void UserAuthManager::addUserAuth(UserAuth::Ptr const& userAuth)
{
    UserAuth::Ptr p = findUserAuth(userAuth->key);
    if(p)
        p->auth = userAuth->auth;
    else
        userAuths_ << userAuth;
}

void UserAuthManager::delUserAuth(QString const& key)
{
    for(int i = 0; i < userAuths_.size(); i++)
    {
        if(userAuths_[i]->key == key)
        {
            userAuths_.removeAt(i);
            break;
        }
    }
}

bool UserAuthManager::save()
{
    QString fileName = QString("%1/UserAuths.json").arg(Utils::sshUserAuthPath());

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly))
        return false;

    QJsonArray userAuths;
    for(int i = 0; i < userAuths_.size(); i++)
    {
        QJsonObject userAuth;
        userAuth.insert("key", userAuths_[i]->key);
        userAuth.insert("auth", encryptText(userAuths_[i]->auth));
        userAuths.append(userAuth);
    }

    QJsonObject docObject;
    docObject.insert("version", "1.0");
    docObject.insert("userAuths", userAuths);

    QJsonDocument doc;
    doc.setObject(docObject);
    file.write(doc.toJson());

    return true;
}

bool UserAuthManager::load()
{
    QString fileName = QString("%1/UserAuths.json").arg(Utils::sshUserAuthPath());

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError json_error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &json_error);
    if(json_error.error != QJsonParseError::NoError)
        return false;

    QJsonArray userAuths = doc.object().value("userAuths").toArray();
    for(int i = 0; i < userAuths.size(); i++)
    {
        QJsonObject object = userAuths.at(i).toObject();
        UserAuth::Ptr userAuth(new UserAuth);
        userAuth->key = object.value("key").toString();
        userAuth->auth = decryptText(object.value("auth").toString());
        userAuths_ << userAuth;
    }

    return true;
}
