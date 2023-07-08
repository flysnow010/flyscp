#ifndef USERAUTH_H
#define USERAUTH_H
#include <QObject>
#include <QString>
#include <QList>
#include <memory>

struct UserAuth
{
    QString key;
    QString auth;
    static QString hash(QString const& text);
    using Ptr = std::shared_ptr<UserAuth>;
};

class UserAuthManager : public QObject
{
public:
    UserAuthManager(QObject* parnet= nullptr);

    UserAuth::Ptr findUserAuth(QString const& key);
    void addUserAuth(UserAuth::Ptr const& userAuth);
    void delUserAuth(QString const& key);

    bool save();
    bool load();
private:
    QList<UserAuth::Ptr> userAuths_;
};

#endif // USERAUTH_H
