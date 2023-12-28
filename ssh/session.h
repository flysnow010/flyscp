#ifndef SSH_SESSION_H
#define SSH_SESSION_H
#include <memory>

namespace ssh {
class SessionPrivate;

typedef unsigned short AuthMethod;
AuthMethod const AuthMethod_Unknow      = 0x0000;
AuthMethod const AuthMethod_None        = 0x0001;
AuthMethod const AuthMethod_PassWord    = 0x0002;
AuthMethod const AuthMethod_PubKey      = 0x0004;
AuthMethod const AuthMethod_Interactive = 0x0008;

inline bool isUnknowOfAuthMethod(AuthMethod method)
{
    return (method & AuthMethod_Unknow) > 0;
}

inline bool isNoneOfAuthMethod(AuthMethod method)
{
    return (method & AuthMethod_None) > 0;
}

inline bool isPasswordOfAuthMethod(AuthMethod method)
{
    return (method & AuthMethod_PassWord) > 0;
}

inline bool isPubKeyOfAuthMethod(AuthMethod method)
{
    return (method & AuthMethod_PubKey) > 0;
}

inline bool isInteractiveOfAuthMethod(AuthMethod method)
{
    return (method & AuthMethod_Interactive) > 0;
}

class Session
{
public:
    Session();
    ~Session();

    typedef std::shared_ptr<Session> Ptr;
    void set_host(const char* host);
    void set_port(int port);
    void set_port(const char* port);
    void set_user(const char* user);

    bool connect(int timeout = 10);
    void disconnect();
    bool verify();

    AuthMethod auth_method(const char *username);

    bool login(const char* password);
    bool login(const char* user, const char* password);
    bool login_by_interactive(const char* user, const char* password);
    bool login_by_prikey(const char* user, const char* privkeyfile);
    bool login_by_pubkey(const char* user, const char* pubkeyfile);
    const char* error();
private:
    friend class Channel;
    friend class Scp;
    friend class SFtp;
    friend class File;
    Session(Session const&);
    Session & operator == (Session const&);

    SessionPrivate* d;
};
}
#endif // SSH_SESSION_H
