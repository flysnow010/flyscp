#include "scp.h"
#include "dir.h"
#include "session.h"
#include "sshprivate.h"
#include<sys/stat.h>
#include <iostream>

namespace ssh {

Scp::Scp(Session const& session)
    : d(new ScpPrivate(session.d->session))
{
}

Scp::~Scp()
{
    close();
    ssh_scp_free(d->scp);
    delete d;
}

bool Scp::open(const char* filepath, bool isRead)
{
    if(d->isOpened)
        return false;

    if(isRead)
        d->scp = ssh_scp_new(d->session, SSH_SCP_READ, filepath);
    else
        d->scp = ssh_scp_new(d->session, SSH_SCP_WRITE | SSH_SCP_RECURSIVE, filepath);
    if(!d->scp)
        return false;
    if(ssh_scp_init(d->scp) != SSH_OK)
    {
        ssh_scp_free(d->scp);
        return false;
    }
    d->isOpened = true;
    return true;
}

void Scp::close()
{
    if(d->isOpened)
    {
        ssh_scp_close(d->scp);
        d->isOpened = false;
    }
}

int Scp::pull_req()
{
    return ssh_scp_pull_request(d->scp);
}

size_t Scp::get_size()
{
    return ssh_scp_request_get_size64(d->scp);
}

const char* Scp::get_filename()
{
    return ssh_scp_request_get_filename(d->scp);
}

const char* Scp::get_warning()
{
    return ssh_scp_request_get_warning(d->scp);
}

bool Scp::accept_req()
{
    if(ssh_scp_accept_request(d->scp) != SSH_OK)
        return false;
    return true;
}

bool Scp::deny_req(const char *reason)
{
    if(ssh_scp_deny_request(d->scp, reason) != SSH_OK)
        return false;
    return true;
}

int Scp::get_permissions()
{
    return ssh_scp_request_get_permissions(d->scp);
}

ssize_t Scp::read(void *buf, size_t count)
{
    ssize_t size = ssh_scp_read(d->scp, buf, count);
    if(size == -1)
    {
        std::cout << ssh_get_error(d->session) << std::endl;
    }
    return size;
}
ssize_t Scp::write(const void *buf, size_t count)
{
    return ssh_scp_write(d->scp, buf, count);
}

bool Scp::mkdir(const char* dirname)
{
    if(ssh_scp_push_directory(d->scp, dirname, S_IRWXU)  != SSH_OK)
        return false;
    return true;
}
bool Scp::mkfile(const char* filename, size_t count)
{
    if(ssh_scp_push_file64(d->scp, filename, count, S_IRUSR |  S_IWUSR)  != SSH_OK)
        return false;
    return true;
}

bool Scp::leave_dir()
{
    if(ssh_scp_leave_directory(d->scp) != SSH_OK)
        return false;
    return true;
}

DirPtr Scp::home() const
{
    return DirPtr(new Dir(*this, "."));
}
DirPtr Scp::root() const
{
    return DirPtr(new Dir(*this, "/"));
}
DirPtr Scp::dir(const char* path) const
{
    return DirPtr(new Dir(*this, path));
}

}
