#include "fileinfo.h"
#include "sshprivate.h"
#include <libssh/sftp.h>

namespace ssh {
FileInfo::FileInfo()
    : d(new FileInfoPrivate())
{
}

FileInfo::~FileInfo()
{
    if(d->info)
        sftp_attributes_free(d->info);
    delete d;
}


bool FileInfo::isNull() const
{
    return  d->info == 0;
}

const char* FileInfo::name()
{
    if(isNull())
        return 0;

    if(d->info->name)
        return d->info->name;
    return d->temp_.c_str();
}

const char* FileInfo::longname()
{
    if(isNull())
        return 0;

    if(d->info->longname)
        return d->info->longname;
    return d->temp_.c_str();
}

uint64_t FileInfo::size()
{
    if(isNull())
        return 0;
    return d->info->size;
}

}
