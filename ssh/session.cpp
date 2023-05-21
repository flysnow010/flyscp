#include "session.h"
#include "sshprivate.h"
#include <libssh/libssh.h>
namespace ssh {

Session::Session()
    : d(new SessionPrivate(ssh_new()))
{
    ssh_options_set(d->session, SSH_OPTIONS_HOSTKEYS, "ssh-rsa");
    ssh_options_set(d->session, SSH_OPTIONS_PUBLICKEY_ACCEPTED_TYPES, "ssh-rsa");
}
Session::~Session()
{
    ssh_free(d->session);
    delete d;
}

void Session::set_host(const char* host)
{
     ssh_options_set(d->session, SSH_OPTIONS_HOST, host);
}

void Session::set_port(int port)
{
    ssh_options_set(d->session, SSH_OPTIONS_PORT, &port);
}

void Session::set_port(const char* port)
{
    ssh_options_set(d->session, SSH_OPTIONS_PORT_STR, port);
}

void Session::set_user(const char* user)
{
    ssh_options_set(d->session, SSH_OPTIONS_USER, user);
}

bool Session::connect()
{
    return ssh_connect(d->session) == SSH_OK;
}

void Session::disconnect()
{
    ssh_disconnect(d->session);
}

bool Session::verify()
{
    ssh_key srv_pubkey = NULL;
    int rc = ssh_get_server_publickey(d->session, &srv_pubkey);
    if(rc < 0)
        return false;

    unsigned char *hash = NULL;
    size_t hlen;
    rc = ssh_get_publickey_hash(srv_pubkey, SSH_PUBLICKEY_HASH_SHA1,
                                &hash, &hlen);
    ssh_key_free(srv_pubkey);
    if(rc < 0)
        return false;
    enum ssh_known_hosts_e state = ssh_session_is_known_server(d->session);
    ssh_clean_pubkey_hash(&hash);
    if(state != SSH_KNOWN_HOSTS_OK && state != SSH_KNOWN_HOSTS_OTHER)
        return false;
    return true;
}

bool Session::login(const char* password)
{
    return login(0, password);
}

bool Session::login(const char* user, const char* password)
{
    return ssh_userauth_password(d->session, user, password) == SSH_AUTH_SUCCESS;
}

const char* Session::error()
{
    return ssh_get_error(d->session);
}
}
