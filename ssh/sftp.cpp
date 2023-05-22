#include "sftp.h"
#include "session.h"
#include "channel.h"
#include "dir.h"
#include "sshprivate.h"

#include <sys/stat.h>

namespace ssh {

SFtp::SFtp(Session const& session)
    : d(new SFtpPrivate(session.d->session))
{
}

SFtp::SFtp(Session const& session, Channel const& channel)
    : d(new SFtpPrivate(session.d->session, channel.d->channel))
{
    ssh_channel_open_session(d->channel);
    ssh_channel_request_sftp(d->channel);
}

SFtp::~SFtp()
{
    if(d->isInited)
        sftp_free(d->sftp);
    delete d;
}

bool SFtp::init()
{
    if(d->isInited)
        return false;

    if(sftp_init(d->sftp) != SSH_OK)
        return false;
    d->isInited = true;
    return true;
}

unsigned int SFtp::extension_count()
{
    return sftp_extensions_get_count(d->sftp);
}

const char * SFtp::extension_name(unsigned int index)
{
    return sftp_extensions_get_name(d->sftp, index);
}

const char * SFtp::extension_data(unsigned int index)
{
    return sftp_extensions_get_data(d->sftp, index);
}

bool SFtp::mkdir(const char* dirname)
{
    if(sftp_mkdir(d->sftp, dirname, S_IRWXU) != SSH_OK)
        return false;
    return true;
}

DirPtr SFtp::home() const
{
    return DirPtr(new Dir(*this, "."));
}

DirPtr SFtp::root() const
{
    return DirPtr(new Dir(*this, "/"));
}

DirPtr SFtp::dir(const char* path) const
{
    return DirPtr(new Dir(*this, path));
}

const char* SFtp::error()
{
    return ssh_get_error(d->session);
}
}
