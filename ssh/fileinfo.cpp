#include "fileinfo.h"
#include "sshprivate.h"
#include <libssh/sftp.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

namespace ssh {
FileInfo::FileInfo()
    : d(new FileInfoPrivate())
{
}

FileInfo::FileInfo(const char* longname)
    : d(new FileInfoPrivate(longname))
{
    d->parse();
}


#define NEXT_CHAR(s) \
    do {             \
        s++;         \
        if(*s == 0)  \
            return;  \
    }while(0)

#define LAST_CHAR(s) \
    do {             \
        s++;         \
        if(*s == 0)  \
            break;  \
    }while(0)

#define SKIP_SPACES(s) \
    while(*s == ' ') \
        NEXT_CHAR(s)

#define SKIP_TEXT(s, c) \
    while(*s != c) \
        LAST_CHAR(s)

char *strndup(const char *s, const char *e)
{
    char* str = (char*)calloc(1, e - s + 1);
    char* d = str;
    while(s != e)
        *d++ = *s++;
    return str;
}

void FileInfoPrivate::parse()
{
    const char* s = info->longname;
    if(!s || !s[0])
        return;

    if(*s == 'd')
        info->type = FileType_Dir;
    else if(*s == 'l')
        info->type = FileType_SymLink;
    else if(*s == '-')
        info->type = FileType_File;

    NEXT_CHAR(s);
    if(*s == 'r')
        info->permissions |= FileInfo::User_Read;
    NEXT_CHAR(s);
    if(*s == 'w')
        info->permissions |= FileInfo::User_Write;
    NEXT_CHAR(s);
    if(*s == 'x')
        info->permissions |= FileInfo::User_Exe;

    NEXT_CHAR(s);
    if(*s == 'r')
        info->permissions |= FileInfo::Group_Read;
    NEXT_CHAR(s);
    if(*s == 'w')
        info->permissions |= FileInfo::Group_Write;
    NEXT_CHAR(s);
    if(*s == 'x')
        info->permissions |= FileInfo::Group_Exe;

    NEXT_CHAR(s);
    if(*s == 'r')
        info->permissions |= FileInfo::Other_Read;
    NEXT_CHAR(s);
    if(*s == 'w')
        info->permissions |= FileInfo::Other_Write;
    NEXT_CHAR(s);
    if(*s == 'x')
        info->permissions |= FileInfo::Other_Exe;

    NEXT_CHAR(s);

    SKIP_SPACES(s);
    SKIP_TEXT(s, ' ');

    SKIP_SPACES(s);
    const char* p = s;
    SKIP_TEXT(s, ' ');
    info->owner = strndup(p, s);

    SKIP_SPACES(s);
    p = s;
    SKIP_TEXT(s, ' ');
    info->group = strndup(p, s);

    SKIP_SPACES(s);
    p = s;
    SKIP_TEXT(s, ' ');
    info->size = strtoull(p, 0, 10);

    struct tm  t;

    SKIP_SPACES(s); //1970-01-01
    p = s;
    t.tm_year = strtoull(p, 0, 10) - 1900;
    SKIP_TEXT(s, '-');
    NEXT_CHAR(s);
    p = s;
    t.tm_mon = strtoull(p, 0, 10) - 1;
    SKIP_TEXT(s, '-');
    NEXT_CHAR(s);
    p = s;
    t.tm_mday = strtoull(p, 0, 10);
    SKIP_TEXT(s, ' ');


    SKIP_SPACES(s); //00:00:33
    p = s;
    t.tm_hour = strtoull(p, 0, 10);
    SKIP_TEXT(s, ':');
    NEXT_CHAR(s);
    p = s;
    t.tm_min = strtoull(p, 0, 10);
    SKIP_TEXT(s, ':');
    NEXT_CHAR(s);
    p = s;
    t.tm_sec = strtoull(p, 0, 10);
    SKIP_TEXT(s, ' ');

    t.tm_isdst = 0;
    t.tm_wday = 0;
    t.tm_yday = 0;
    info->mtime = mktime(&t);

    SKIP_SPACES(s); //+0000
    SKIP_TEXT(s, ' ');

    SKIP_SPACES(s); //log
    p = s;
    SKIP_TEXT(s, ' ');
    info->name = strndup(p, s);
}
//drwxr-xr-x    2 root     root           160 1970-01-01 00:00:33 +0000 log

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

bool FileInfo::otherCanRead() const
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

}
