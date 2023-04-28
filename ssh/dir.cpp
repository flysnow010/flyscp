#include "dir.h"
#include "sftp.h"
#include "fileinfo.h"
#include "sshprivate.h"

namespace ssh {
Dir::Dir(SFtp const& sftp, const char* path)
    : d(new DirPrivate(sftp.d->sftp, path))
{
}

Dir::~Dir()
{
    delete d;
}

const char* Dir::dirName()
{
    return d->path.c_str();
}

FileInfos Dir::fileInfoList()
{
    FileInfos fileInfos;
    sftp_dir dir = sftp_opendir(d->sftp, d->path.c_str());
    if(!dir)
        return fileInfos;

    sftp_attributes info;
    while((info = sftp_readdir(d->sftp, dir)))
    {
        FileInfoPtr fileInfo(new FileInfo());
        fileInfo->d->info = info;
        fileInfos.push_back(fileInfo);
    }
    sftp_closedir(dir);
    return fileInfos;
}

FileInfoPtr Dir::fileInfo(const char* path)
{
    sftp_attributes info = sftp_stat(d->sftp, path);
    FileInfoPtr fileInfo(new FileInfo());
    fileInfo->d->info = info;
    return fileInfo;
}

}
