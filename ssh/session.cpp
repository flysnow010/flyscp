#include "session.h"
#include "sshprivate.h"
#include <libssh/libssh.h>
namespace ssh {

#define HOSTKEYS "ssh-ed25519," \
                 "ecdsa-sha2-nistp521," \
                 "ecdsa-sha2-nistp384," \
                 "ecdsa-sha2-nistp256," \
                 "sk-ssh-ed25519@openssh.com," \
                 "sk-ecdsa-sha2-nistp256@openssh.com," \
                 "rsa-sha2-512," \
                 "rsa-sha2-256," \
                 "ssh-rsa"

Session::Session()
    : d(new SessionPrivate(ssh_new()))
{
    ssh_options_set(d->session, SSH_OPTIONS_HOSTKEYS, HOSTKEYS);
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

bool Session::connect(int timeout)
{
    ssh_options_set(d->session, SSH_OPTIONS_TIMEOUT, &timeout);
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
    if(state != SSH_KNOWN_HOSTS_OK
            && state != SSH_KNOWN_HOSTS_UNKNOWN
            && state != SSH_KNOWN_HOSTS_OTHER
            && state != SSH_KNOWN_HOSTS_NOT_FOUND)
        return false;
    return true;
}

AuthMethod Session::auth_method(const char *username)
{
    AuthMethod method = AuthMethod_Unknow;
    if(ssh_userauth_none(d->session, username) == SSH_AUTH_ERROR)
        return method;

    int mask = ssh_userauth_list(d->session, 0);
    if(mask & SSH_AUTH_METHOD_NONE)
        method |= AuthMethod_None;
    if(mask & SSH_AUTH_METHOD_PASSWORD)
        method |= AuthMethod_PassWord;
    if(mask & SSH_AUTH_METHOD_PUBLICKEY)
        method |= AuthMethod_PubKey;
    if(mask & SSH_AUTH_METHOD_INTERACTIVE)
        method |= AuthMethod_Interactive;
    return method;
}

bool Session::login(const char* password)
{
    return login(0, password);
}

bool Session::login(const char* user, const char* password)
{
    return ssh_userauth_password(d->session, user, password) == SSH_AUTH_SUCCESS;
}

bool Session::login_by_interactive(const char* user,
                                   const char* password)
{
    int err = ssh_userauth_kbdint(d->session, user, 0);
    while (err == SSH_AUTH_INFO) {
        const char *instruction;
        const char *name;
        char buffer[128];
        int i, n;

        name = ssh_userauth_kbdint_getname(d->session);
        instruction = ssh_userauth_kbdint_getinstruction(d->session);
        n = ssh_userauth_kbdint_getnprompts(d->session);
        if (name && strlen(name) > 0) {
            printf("%s\n", name);
        }

        if (instruction && strlen(instruction) > 0) {
            printf("%s\n", instruction);
        }
        for (i = 0; i < n; i++) {
            const char *answer;
            const char *prompt;
            char echo;

            prompt = ssh_userauth_kbdint_getprompt(d->session, i, &echo);
            if (prompt == NULL) {
                break;
            }

            if (echo) {
                char *p;

                printf("%s", prompt);

                if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                    return false;
                }

                buffer[sizeof(buffer) - 1] = '\0';
                if ((p = strchr(buffer, '\n'))) {
                    *p = '\0';
                }

                if (ssh_userauth_kbdint_setanswer(d->session, i, buffer) < 0) {
                    return false;
                }

                memset(buffer, 0, strlen(buffer));
            } else {
                if (password && strstr(prompt, "Password:")) {
                    answer = password;
                } else {
                    buffer[0] = '\0';

                    if (ssh_getpass(prompt, buffer, sizeof(buffer), 0, 0) < 0) {
                        return false;
                    }
                    answer = buffer;
                }
                err = ssh_userauth_kbdint_setanswer(d->session, i, answer);
                memset(buffer, 0, sizeof(buffer));
                if (err < 0) {
                    return false;
                }
            }
        }
        err = ssh_userauth_kbdint(d->session, 0, 0);
    }
    return err == SSH_AUTH_SUCCESS;
}

bool Session::login_by_prikey(const char* user, const char* privkeyfile)
{
    ssh_key key = 0;
    int r = ssh_pki_import_privkey_file(privkeyfile, 0, 0, 0, &key);
    if(r != SSH_AUTH_SUCCESS)
        return false;
    r = ssh_userauth_publickey(d->session, user, key);
    ssh_key_free(key);
    return r == SSH_OK;
}

bool Session::login_by_pubkey(const char* user, const char* pubkeyfile)
{
    ssh_key key = 0;
    int r = ssh_pki_import_pubkey_file(pubkeyfile, &key);
    if(r != SSH_AUTH_SUCCESS)
        return false;
    r = ssh_userauth_try_publickey(d->session, user, key);
    ssh_key_free(key);
    return r == SSH_OK;
}

const char* Session::error()
{
    return ssh_get_error(d->session);
}
}
