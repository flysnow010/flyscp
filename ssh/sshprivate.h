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
    DirPrivate(const char * p)
        : path(p)
    {}

    virtual ~DirPrivate() {}

    virtual bool opendir() = 0;
    virtual sftp_attributes readdir() = 0;
    virtual void closedir() = 0;

    virtual bool mkdir(const char* path) = 0;
    virtual bool rmdir(const char* path) = 0;
    virtual bool mkfile(const char* filename) = 0;
    virtual bool rmfile(const char* filename) = 0;
    virtual bool rename(const char *original, const  char *newname) = 0;
    virtual bool chmod(const char* filename, uint16_t mode) = 0;

    std::string path;
};

class SftpDirPrivate : public DirPrivate
{
public:
    SftpDirPrivate(const char * p, sftp_session s)
        : DirPrivate(p)
        , sftp(s)
        , dir(0)
    {}

    bool opendir() override;
    sftp_attributes readdir() override;
    void closedir() override;

    bool mkdir(const char* path) override;
    bool rmdir(const char* path) override;
    bool mkfile(const char* filename) override;
    bool rmfile(const char* filename) override;
    bool rename(const char *original, const  char *newname) override;
    bool chmod(const char* filename, uint16_t mode) override;

    sftp_session sftp;
    sftp_dir dir;
};

class ChannelDirPrivate : public DirPrivate
{
public:
    ChannelDirPrivate(const char * p, ssh_session s)
        : DirPrivate(p)
        , session(s)
        , dirline(0)
    {}

    bool opendir() override;
    sftp_attributes readdir() override;
    void closedir() override;

    bool mkdir(const char* path) override;
    bool rmdir(const char* path) override;
    bool mkfile(const char* filename) override;
    bool rmfile(const char* filename) override;
    bool rename(const char *original, const  char *newname) override;
    bool chmod(const char* filename, uint16_t mode) override;

    bool exec(std::string const& command);
    ssh_session session;
    std::string lstext;
    char* dirline;
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
