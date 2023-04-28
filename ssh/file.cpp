#include "file.h"
#include "sftp.h"
#include  "sshprivate.h"

namespace ssh {
File::File(SFtp const& sftp)
    : d(new FilePrivate(sftp.d->sftp))
{
}

File::~File()
{
    close();
    delete d;
}

bool File::open(const char *file, int accesstype, mode_t mode)
{
    d->file = sftp_open(d->sftp, file, accesstype, mode);
    if(d->file)
        return true;
    return false;
}

bool File::close()
{
    if(d->file)
    {
        sftp_close(d->file);
        d->file = 0;
        return true;
    }
    return false;
}

void File::set_noblocking(bool enable)
{
    if(enable)
        sftp_file_set_nonblocking(d->file);
    else
        sftp_file_set_blocking(d->file);
}

ssize_t File::read(void *buf, size_t count)
{
    return sftp_read(d->file, buf, count);
}

ssize_t File::write(const void *buf, size_t count)
{
    return sftp_write(d->file, buf, count);
}

int File::seek(uint32_t new_offset)
{
    return sftp_seek(d->file, new_offset);
}

int File::seek(uint64_t new_offset)
{
    return sftp_seek64(d->file, new_offset);
}

uint64_t File::tell()
{
    return sftp_tell64(d->file);
}

void File::rewind()
{
    sftp_rewind(d->file);
}

}
