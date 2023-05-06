#include "dir.h"
#include "sftp.h"
#include "fileinfo.h"
#include "sshprivate.h"
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>

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
}
Dir::Dir(SFtp const& sftp, const char* path)
    : d(new DirPrivate(sftp.d->sftp, path))
{
}

Dir::~Dir()
{
    delete d;
}

bool Dir::isRoot() const
{
    return d->path == std::string("/");
}

const char* Dir::dirName() const
{
    return d->path.c_str();
}

FileInfos Dir::fileInfoList(Filter filter, SortFlag sortFlag)
{
    FileInfos fileInfos;
    sftp_dir dir = sftp_opendir(d->sftp, d->path.c_str());
    if(!dir)
        return fileInfos;

    sftp_attributes info;
    while((info = sftp_readdir(d->sftp, dir)))
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
    sftp_closedir(dir);
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

        if(l->isDir())
        {
            if(r->isFile())
            {
                if(flag & Dir::DirsFirst)
                    return true;
                return false;
            }
            else
            {
                if(l->isParent())
                    return true;
                if(r->isParent())
                    return false;
                if(flag & Dir::Reversed)
                    return compare(l, r);
                return compare(r, l);
            }
        }
        else
        {
            if(r->isDir())
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
    if(sftp_mkdir(d->sftp, path, S_IRWXU) != SSH_OK)
        return false;
    return true;
}

bool Dir::rmdir(const char* path)
{
    if(sftp_rmdir(d->sftp, path) != SSH_OK)
        return false;
    return true;
}


bool Dir::mkFile(const char* filename)
{
    sftp_file file = sftp_open(d->sftp, filename, O_CREAT | O_EXCL, 0644);
    if(!file)
        return false;
    sftp_close(file);
    return true;
}

bool Dir::rmFile(const char* filename)
{
    if(sftp_unlink(d->sftp, filename) != SSH_OK)
        return false;
    return true;
}

bool Dir::rename(const char *original, const  char *newname)
{
     if(sftp_rename(d->sftp, original, newname) != SSH_OK)
         return false;
     return true;
}

bool Dir::chmod(const char* filename, uint16_t mode)
{
    if(sftp_chmod(d->sftp, filename, mode) != SSH_OK)
        return false;
    return true;
}

}
