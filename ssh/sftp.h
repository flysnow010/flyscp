#ifndef SSH_SFTP_H
#define SSH_SFTP_H
#include <memory>

namespace ssh {
class SFtpPrivate;
class Session;
class Channel;
class Dir;
class DirPtr;
class SFtp
{
public:
    SFtp(Session const& session);
    SFtp(Session const& session, Channel const& channel);
    ~SFtp();

    typedef std::shared_ptr<SFtp> Ptr;
    bool init();

    unsigned int extension_count();
    const char * extension_name(unsigned int index);
    const char * extension_data(unsigned int index);

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
