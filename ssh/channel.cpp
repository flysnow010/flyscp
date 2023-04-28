#include "channel.h"
#include "session.h"
#include "sshprivate.h"
#include <libssh/libssh.h>

namespace ssh {

Channel::Channel(const Session &session)
    : d(new ChannelPrivate(ssh_channel_new(session.d->session)))
{
}

Channel::~Channel()
{
    if(d->isOpened)
        ssh_channel_close(d->channel);
    ssh_channel_free(d->channel);
    delete d;
}

bool Channel::open()
{
    if(d->isOpened)
        return false;

    if(ssh_channel_open_session(d->channel) != SSH_OK)
        return false;
    d->isOpened = true;
    return true;
}

bool Channel::close()
{
    if(!d->isOpened)
        return false;
    ssh_channel_close(d->channel);
    d->isOpened = false;
    return true;
}

bool Channel::sendEof()
{
    return ssh_channel_send_eof(d->channel) == SSH_OK;
}

bool Channel::exec(const char* command)
{
    return ssh_channel_request_exec(d->channel, command) == SSH_OK;
}

int Channel::poll()
{
    return ssh_channel_poll(d->channel, 0);
}

int Channel::poll(int timeout)
{
    return ssh_channel_poll_timeout(d->channel, timeout, 0);
}

int Channel::read(void *dest, uint32_t count)
{
    return ssh_channel_read(d->channel, dest, count, 0);
}

int Channel::read_nonblocking(void *dest, uint32_t count)
{
    return ssh_channel_read_nonblocking(d->channel, dest, count, 0);
}

int Channel::write(void *dest, uint32_t count)
{
    return ssh_channel_write(d->channel, dest, count);
}

bool Channel::runShell(int cols, int rows)
{
    if (ssh_channel_request_pty(d->channel) != SSH_OK)
        return false;

    if (ssh_channel_change_pty_size(d->channel, cols, rows) != SSH_OK)
        return false;

    if (ssh_channel_request_shell(d->channel))
        return false;

    return true;
}

bool Channel::shellIsOpen()
{
    if(ssh_channel_is_open(d->channel))
        return true;
    return false;
}

bool Channel::shellIsEof()
{
    if(ssh_channel_is_eof(d->channel))
        return true;
    return false;
}
}
