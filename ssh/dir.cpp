#include "dir.h"
#include "sftp.h"
#include "scp.h"
#include "fileinfo.h"
#include "channel.h"
#include "sshprivate.h"
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <time.h>

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

#define SKIP_TOKEN(s, c) \
    while(*s != c) \
        LAST_CHAR(s)

namespace ssh {
namespace  {
    void splitBasenameAndSuffix(FileInfoPrivate * fileInfo)
    {
        if(fileInfo->info->type == FileType_Dir)
            fileInfo->basename = std::string(fileInfo->info->name);
        else
        {
            size_t len = strlen(fileInfo->info->name);
            while(len > 0 && fileInfo->info->name[len - 1] != '.') len--;
            if(len == 0)
                fileInfo->basename = std::string(fileInfo->info->name);
            else
            {
                fileInfo->basename = std::string(fileInfo->info->name, len - 1);
                fileInfo->suffix = std::string(fileInfo->info->name + len);
            }
        }
    }

    char *strndup(const char *s, const char *e)
    {
        char* str = (char*)calloc(1, e - s + 1);
        char* d = str;
        while(s != e)
            *d++ = *s++;
        return str;
    }

    void parseFileSttributes(sftp_attributes info)
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

        SKIP_SPACES(s); //+0000
        p = s;
        int timetone = strtoull(p, 0, 10);
        if(timetone == 0)
            t.tm_hour += 8;
        SKIP_TEXT(s, ' ');

        t.tm_isdst = 0;
        t.tm_wday = 0;
        t.tm_yday = 0;
        info->mtime = mktime(&t);

        SKIP_SPACES(s); //log
        p = s;
        SKIP_TEXT(s, ' ');
        info->name = strndup(p, s);
    }
}

bool SftpDirPrivate::opendir()
{
    dir = sftp_opendir(sftp, path.c_str());
    if(dir)
        return true;
    return false;
}

sftp_attributes SftpDirPrivate::readdir()
{
    return sftp_readdir(sftp, dir);
}

void SftpDirPrivate::closedir()
{
    sftp_closedir(dir);
}

bool SftpDirPrivate::mkdir(const char* path)
{
    if(sftp_mkdir(sftp, path, S_IRWXU) != SSH_OK)
        return false;
    return true;
}

bool SftpDirPrivate::rmdir(const char* path)
{
    if(sftp_rmdir(sftp, path) != SSH_OK)
        return false;
    return true;
}

bool SftpDirPrivate::mkfile(const char* filename)
{
    sftp_file file = sftp_open(sftp, filename, O_CREAT | O_EXCL, 0644);
    if(!file)
        return false;
    sftp_close(file);
    return true;
}

bool SftpDirPrivate::rmfile(const char* filename)
{
    if(sftp_unlink(sftp, filename) != SSH_OK)
        return false;
    return true;
}

bool SftpDirPrivate::rename(const char *original, const  char *newname)
{
    if(sftp_rename(sftp, original, newname) != SSH_OK)
        return false;
    return true;
}

bool SftpDirPrivate::chmod(const char* filename, uint16_t mode)
{
    if(sftp_chmod(sftp, filename, mode) != SSH_OK)
        return false;
    return true;
}

#define EXEC_COMMAND(command) \
    ssh_channel channel = ssh_channel_new(session); \
    if(!channel) \
        return false;\
    if(ssh_channel_open_session(channel) != SSH_OK)\
    {\
        ssh_channel_free(channel);\
        return false;\
    }\
    if(ssh_channel_request_exec(channel, command.c_str()) != SSH_OK)\
    {\
        ssh_channel_close(channel);\
        ssh_channel_free(channel);\
        return false;\
    }

#define END_COMMAND \
    do {\
        ssh_channel_send_eof(channel); \
        ssh_channel_close(channel); \
        ssh_channel_free(channel);\
    }while(false)


bool ChannelDirPrivate::opendir()
{
    std::string command = std::string("ls -la --full-time ")  + path;
    EXEC_COMMAND(command)

    while(true)
    {
        char buffer[256];
        int bytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
        if(bytes <= 0)
            break;
        lstext += std::string(buffer, bytes);
    }
    dirline = strtok((char *)lstext.c_str(), "\n");
    END_COMMAND;
    return true;
}

sftp_attributes ChannelDirPrivate::readdir()
{
    dirline = strtok(0, "\n");
    if(!dirline)
        return 0;

    sftp_attributes info = (sftp_attributes)calloc(1, sizeof(*info));
    info->longname = strdup(dirline);
    parseFileSttributes(info);
    return info;
}

void ChannelDirPrivate::closedir()
{
    dirline = 0;
    lstext.clear();
}

bool ChannelDirPrivate::mkdir(const char* path)
{
    return exec(std::string("mkdir ") + path);
}

bool ChannelDirPrivate::rmdir(const char* path)
{
    return exec(std::string("rm -r ") + path);
}

bool ChannelDirPrivate::mkfile(const char* filename)
{
    return exec(std::string("touch ") + filename);
}

bool ChannelDirPrivate::rmfile(const char* filename)
{
    return exec(std::string("rm ") + filename);
}

bool ChannelDirPrivate::rename(const char *original, const  char *newname)
{
    std::string command = std::string("mv ") + original
            + std::string(" ") + newname;
    return exec(command);
}

bool ChannelDirPrivate::chmod(const char* filename, uint16_t mode)
{
    char strMode[64];
    snprintf(strMode, sizeof(strMode), "%o", mode);
    std::string command = std::string("chmod  ") + std::to_string(mode)
            + std::string(" ") + filename;
    return exec(command);
}

bool ChannelDirPrivate::exec(std::string const& command)
{
    EXEC_COMMAND(command)
    END_COMMAND;
    return true;
}

Dir::Dir(SFtp const& sftp, const char* path)
    : d(new SftpDirPrivate(path, sftp.d->sftp))
{
}

Dir::Dir(Scp const& scp, const char* path)
    : d(new ChannelDirPrivate(path, scp.d->session))
{
}

Dir::~Dir()
{
    delete d;
}

bool Dir::is_root() const
{
    return d->path == std::string("/");
}

const char* Dir::dirname() const
{
    return d->path.c_str();
}

FileInfos Dir::fileinfos(Filter filter, SortFlag sortFlag)
{
    FileInfos fileInfos;
    if(!d->opendir())
        return fileInfos;

    sftp_attributes info;
    while((info = d->readdir()))
    {
        uint32_t typeMask = filter & TypeMask;
        if(info->type == FileType_File)
        {
            if(!(typeMask & Files))
                continue;
        }

        if(info->type == FileType_Dir)
        {
            if(!(typeMask & Dirs))
                continue;
        }

        if(info->type == FileType_SymLink)
        {
            if(typeMask & NoSymLinks)
                continue;
        }

        if(!strcmp(info->name, "."))
        {
            if(filter & NoDot)
                continue;
        }
        bool isParent = false;
        if(!strcmp(info->name, ".."))
        {
            if(filter & NoDotDot)
                continue;
            isParent = true;
        }

        typeMask = filter & AccessMask;
        if(strlen(info->name) > 2
                && info->name[0] == '.'
                && info->name[1] != '.')
        {
            if(!(typeMask & Hidden))
                continue;
        }

        FileInfoPtr fileInfo(new FileInfo());
        fileInfo->d->info = info;
        fileInfo->d->isParent = isParent;
        splitBasenameAndSuffix(fileInfo->d);
        fileInfos.push_back(fileInfo);
    }
    d->closedir();
    sort(fileInfos, sortFlag);
    return fileInfos;
}

struct FileInfoCompare
{
    FileInfoCompare(Dir::SortFlag f)
        : flag(f)
    {}

    bool operator() (FileInfoPtr const& l, FileInfoPtr const& r)
    {
        if(flag == Dir::NoSort)
            return false;

        if(l->is_dir())
        {
            if(r->is_file())
            {
                if(flag & Dir::DirsFirst)
                    return true;
                return false;
            }
            else
            {
                if(l->is_parent())
                    return true;
                if(r->is_parent())
                    return false;
                if(flag & Dir::Reversed)
                    return compare(l, r);
                return compare(r, l);
            }
        }
        else
        {
            if(r->is_dir())
            {
                if(flag & Dir::DirsFirst)
                    return false;
                return true;
            }
            else
            {
                if(flag & Dir::Reversed)
                    return compare(l, r);
                return compare(r, l);
            }
        }
    }

    bool compare(FileInfoPtr const& l, FileInfoPtr const& r)
    {
        uint32_t sort = flag & Dir::SortByMask;
        if(sort == Dir::Name)
            return l->basename() < r->basename();
        else if(sort == Dir::Time)
            return l->time() < r->time();
        else if(sort == Dir::Size)
            return l->size() < r->size();
        else if(sort == Dir::Type)
            return l->suffix() < r->suffix();
        else if(sort == Dir::Property)
            return l->permissions() < r->permissions();
        else
            return false;
    }

    Dir::SortFlag flag;
};

void Dir::sort(FileInfos &fileInfos, SortFlag sortFlag)
{
    std::sort(fileInfos.begin(), fileInfos.end(), FileInfoCompare(sortFlag));
}

bool Dir::mkdir(const char*path)
{
    return d->mkdir(path);
}

bool Dir::rmdir(const char* path)
{
    return d->rmdir(path);
}


bool Dir::mkfile(const char* filename)
{
    return d->mkfile(filename);
}

bool Dir::rmfile(const char* filename)
{
    return d->rmfile(filename);
}

bool Dir::rename(const char *original, const  char *newname)
{
     return d->rename(original, newname);
}

bool Dir::chmod(const char* filename, uint16_t mode)
{
    return d->chmod(filename, mode);
}

std::string Dir::dirname(const char* filename)
{
    char* name = ssh_dirname(filename);
    std::string dir(name);
    SSH_STRING_FREE_CHAR(name);
    return dir;
}

std::string Dir::basename(const char* filename)
{
    char* name = ssh_basename(filename);
    std::string base(name);
    SSH_STRING_FREE_CHAR(name);
    return base;
}

}
