#ifndef SSH_SESSION_H
#define SSH_SESSION_H
#include <memory>

namespace ssh {
class SessionPrivate;
class Session
{
public:
    Session();
    ~Session();

    typedef std::shared_ptr<Session> Ptr;
    void setHost(const char* host);
    void setPort(int port);
    void setPort(const char* port);
    void setUser(const char* user);

    bool connect();
    void disconnect();
    bool verify();
    bool login(const char* password);
    bool login(const char* user, const char* password);
    const char* error();
private:
    friend class Channel;
    friend class Scp;
    friend class SFtp;
    Session(Session const&);
    Session & operator == (Session const&);

    SessionPrivate* d;
};
}
#endif // SSH_SESSION_H
