#ifndef SSH_SFTP_H
#define SSH_SFTP_H

namespace ssh {
class SFtpPrivate;
class Session;
class Dir;
class DirPtr;
class SFtp
{
public:
    SFtp(Session const& session);
    ~SFtp();

    bool init();

    unsigned int extension_count();
    const char * extension_name(unsigned int index);
    const char * extension_data(unsigned int index);

    void ls();
    DirPtr home() const;
    DirPtr root() const;
    DirPtr dir(const char* path) const;
    bool mkdir(const char*path);
private:
    friend class Dir;
    friend class File;
    SFtp(SFtp const&);
    SFtp & operator == (SFtp const&);

    SFtpPrivate* d;
};

}
#endif // SSH_SFTP_H
