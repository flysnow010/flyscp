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


bool FileInfo::is_null() const
{
    return  d->info == 0;
}

bool FileInfo::is_dir() const
{
    return d->info->type == FileType_Dir;
}

bool FileInfo::is_file() const
{
    return d->info->type == FileType_File;
}

bool FileInfo::is_symlink() const
{
    return d->info->type == FileType_SymLink;
}

bool FileInfo::is_parent() const
{
    return d->isParent;
}

bool FileInfo::other_is_only_read() const
{
    return d->info->permissions & Other_Read;
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

FileInfoPtr FileInfos::find(std::string const& name) const
{
    for(auto it = begin(); it != end(); ++it)
    {
        if(strcmp((*it)->name(), name.c_str()) == 0)
            return *it;
    }
    return FileInfoPtr();
}

}
