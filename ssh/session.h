#ifndef SSH_SESSION_H
#define SSH_SESSION_H

namespace ssh {
class SessionPrivate;
class Session
{
public:
    Session();
    ~Session();

    void setHost(const char* host);
    void setPort(int port);
    void setPort(const char* port);
    void setUser(const char* user);

    bool connect();
    bool verify();
    bool login(const char* password);
    bool login(const char* user, const char* password);
private:
    friend class Channel;
    friend class SFtp;
    Session(Session const&);
    Session & operator == (Session const&);

    SessionPrivate* d;
};
}
#endif // SSH_SESSION_H
