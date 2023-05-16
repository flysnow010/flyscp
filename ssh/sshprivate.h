#ifndef SSHPRIVATE_H
#define SSHPRIVATE_H
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <string>
namespace ssh {
class SessionPrivate
{
public:
    SessionPrivate(ssh_session s)
        : session(s)
    {}

    ssh_session session;
};

class ChannelPrivate
{
public:
    ChannelPrivate(ssh_channel c)
        : channel(c)
        , isOpened(false)
    {}

    ssh_channel channel;
    bool isOpened;
};

class ScpPrivate
{
public:
    ScpPrivate(ssh_session s)
        : session(s)
        , scp(0)
        , isOpened(false)
    {}

   ssh_session session;
   ssh_scp scp;
    bool isOpened;
};


class SFtpPrivate
{
public:
    SFtpPrivate(ssh_session s)
        : session(s)
        , sftp(sftp_new(s))
        , isInited(false)
    {
        channel =  sftp->channel;
    }

    SFtpPrivate(ssh_session s, ssh_channel c)
        : session(s)
        , channel(c)
        , sftp(sftp_new_channel(s, c))
        , isInited(false)
    {}

    ssh_session session;
    ssh_channel channel;
    sftp_session sftp;
    bool isInited;
};

class DirPrivate
{
public:
    DirPrivate(sftp_session s, const char * p)
        : sftp(s)
        , dir(0)
        , channel(0)
        , path(p)
    {}

    DirPrivate(ssh_channel c, const char * p)
        : sftp(0)
        , dir(0)
        , channel(c)
        , path(p)
    {}

    sftp_session sftp;
    sftp_dir dir;
    ssh_channel channel;
    std::string path;
};
class FilePrivate
{
public:
    FilePrivate(sftp_session s)
        : sftp(s)
        , file(0)
    {}

    sftp_session sftp;
    sftp_file file;
};

class FileInfoPrivate
{
public:
    FileInfoPrivate()
        : info(0)
        , isParent(false)
    {}

    FileInfoPrivate(const char* longname)
        : info((sftp_attributes)calloc(1, sizeof(*info)))
        , isParent(false)
    {
        info->longname = strdup(longname);
    }
    void parse();
    sftp_attributes info;
    bool isParent;
    std::string basename;
    std::string suffix;
};

int const FileType_File    = 1;
int const FileType_Dir     = 2;
int const FileType_SymLink = 3;
};
#endif // SSHPRIVATE_H
