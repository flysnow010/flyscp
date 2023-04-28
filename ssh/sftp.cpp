#include "sftp.h"
#include "session.h"
#include "dir.h"
#include "sshprivate.h"
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>
namespace ssh {

SFtp::SFtp(Session const& session)
    : d(new SFtpPrivate(session.d->session))
{
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

bool SFtp::mkdir(const char*path)
{
    if(!d->isInited)
        return false;
    if(sftp_mkdir(d->sftp, path, S_IRWXU) != SSH_OK)
        return false;
    return true;
}

DirPtr SFtp::home() const
{
    DirPtr dir(new Dir(*this, "."));
    return dir;
}

DirPtr SFtp::root() const
{
    DirPtr dir(new Dir(*this, "/"));
    return dir;
}

DirPtr SFtp::dir(const char* path) const
{
    DirPtr dir(new Dir(*this, path));
    return dir;
}

}
