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

bool FileInfo::isDir() const
{
    return d->info->type == FileType_Dir;
}

bool FileInfo::isFile() const
{
    return d->info->type == FileType_File;
}

bool FileInfo::isSymLink() const
{
    return d->info->type == FileType_SymLink;
}

bool FileInfo::isParent() const
{
    return d->isParent;
}

const char* FileInfo::name() const
{
    return d->info->name;
}

std::string const& FileInfo::basename() const
{
    return d->basename;
}

std::string const& FileInfo::suffix() const
{
    return d->suffix;
}

const char* FileInfo::owner() const
{
    return d->info->owner;
}

const char* FileInfo::group() const
{
    return d->info->group;
}

const char* FileInfo::longname() const
{
    return d->info->longname;
}

uint32_t FileInfo::uid() const
{
    return d->info->uid;
}

uint32_t FileInfo::gid() const
{
    return d->info->gid;
}


uint32_t FileInfo::permissions() const
{
    return d->info->permissions;
}

uint32_t FileInfo::time() const
{
    return d->info->mtime;
}


uint64_t FileInfo::size() const
{
    return d->info->size;
}

}
