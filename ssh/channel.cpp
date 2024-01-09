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

void Channel::reset()
{
    d->channel = 0;
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

bool Channel::send_eof()
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

void Channel::set_blocking(bool  enable)
{
    ssh_channel_set_blocking(d->channel, enable ? 1 : 0);
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

bool Channel::run_shell(int cols, int rows)
{
    /*
     *
        vt200 as VT220/VT240,
        vt300 as VT320/VT340,
        vt400 as VT420, and
        vt500 as VT510/VT520/VT525.
        xterm
        xterm-256color
    */
    const char* term = "xterm-256color";//vt100 vt400 xterm xterm-256color
    if (ssh_channel_request_pty_size(d->channel, term, cols, rows) != SSH_OK)
        return false;

    if (ssh_channel_request_shell(d->channel))
        return false;

    return true;
}

bool Channel::shell_size(int cols, int rows)
{
    if (ssh_channel_change_pty_size(d->channel, cols, rows) != SSH_OK)
        return false;
    return true;
}

bool Channel::is_open()
{
    if(ssh_channel_is_open(d->channel))
        return true;
    return false;
}

bool Channel::is_eof()
{
    if(ssh_channel_is_eof(d->channel))
        return true;
    return false;
}
}
