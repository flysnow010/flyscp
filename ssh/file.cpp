#include "file.h"
#include "session.h"
#include "sftp.h"
#include "sshprivate.h"
#include <fcntl.h>

namespace ssh {

bool SftpFilePrivate::open(const char *filename, int accesstype, mode_t mode)
{
    file = sftp_open(sftp, filename, accesstype, mode);
    if(file)
        return true;
    return false;
}

bool SftpFilePrivate::close()
{
    if(!file)
        return true;

    sftp_close(file);
    file = 0;
    return true;
}

void SftpFilePrivate::set_noblocking(bool enable)
{
    if(enable)
        sftp_file_set_nonblocking(file);
    else
        sftp_file_set_blocking(file);
}

ssize_t SftpFilePrivate::read(void *buf, size_t count)
{
    return sftp_read(file, buf, count);
}

ssize_t SftpFilePrivate::write(const void *buf, size_t count)
{
    return sftp_write(file, buf, count);
}

int SftpFilePrivate::seek(uint32_t new_offset)
{
    return sftp_seek(file, new_offset);
}

int SftpFilePrivate::seek(uint64_t new_offset)
{
    return sftp_seek64(file, new_offset);
}

uint64_t SftpFilePrivate::tell()
{
    return sftp_tell64(file);
}

void SftpFilePrivate::rewind()
{
    sftp_rewind(file);
}

bool ScpFilePrivate::open(const char *filename, int accesstype, mode_t mode)
{
    bool isRead = (accesstype == O_RDONLY);
    if(isRead)
        scp = ssh_scp_new(session, SSH_SCP_READ, filename);
    else
        scp = ssh_scp_new(session, SSH_SCP_WRITE | SSH_SCP_RECURSIVE, ssh_dirname(filename));
    if(!scp)
        return false;

    if(ssh_scp_init(scp) != SSH_OK)
    {
        ssh_scp_free(scp);
        return false;
    }
    if(isRead)
    {
        if(ssh_scp_pull_request(scp) != SSH_SCP_REQUEST_NEWFILE)
        {
            ssh_scp_close(scp);
            ssh_scp_free(scp);
            return false;
        }
    }
    else if(ssh_scp_push_file64(scp, ssh_basename(filename), filesize, mode) != SSH_OK)
    {
        ssh_scp_close(scp);
        ssh_scp_free(scp);
        return false;
    }
    return true;
}

bool ScpFilePrivate::close()
{
    if(ssh_scp_close(scp) != SSH_OK)
        return false;
    return true;
}
ssize_t ScpFilePrivate::read(void *buf, size_t count)
{
    return ssh_scp_read(scp, buf, count);
}
ssize_t ScpFilePrivate::write(const void *buf, size_t count)
{
    if(ssh_scp_write(scp, buf, count) != SSH_OK)
        return 0;
    return  count;
}


File::File(SFtp const& sftp)
    : d(new SftpFilePrivate(sftp.d->sftp))
{
}


File::File(Session const& session)
    : d(new ScpFilePrivate(session.d->session))
{
}
File::~File()
{
    d->close();
    delete d;
}

void File::set_filesize(uint64_t filesize)
{
    d->filesize = filesize;
}

bool File::open(const char *filename, int accesstype, mode_t mode)
{
    return d->open(filename, accesstype, mode);
}

bool File::close() { return d->close(); }

void File::set_noblocking(bool enable)
{
    d->set_noblocking(enable);
}

ssize_t File::read(void *buf, size_t count)
{
    return d->read(buf, count);
}

ssize_t File::write(const void *buf, size_t count)
{
    return d->write(buf, count);
}

int File::seek(uint32_t new_offset) { return d->seek(new_offset); }
int File::seek(uint64_t new_offset) { return d->seek(new_offset); }
uint64_t File::tell() { return d->tell(); }
void File::rewind() { d->rewind(); }

}
