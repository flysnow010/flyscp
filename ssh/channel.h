#ifndef SSH_CHANNEL_H
#define SSH_CHANNEL_H
#include <cstdint>

namespace ssh {
class ChannelPrivate;
class Session;
class Channel
{
public:
    Channel(Session const& session);
    ~Channel();

    bool open();
    bool close();
    bool sendEof();

    bool runShell(int cols, int rows);
    bool shellIsOpen();
    bool shellIsEof();

    bool exec(const char* command);
    int poll();
    int poll(int timeout);
    int read(void *dest, uint32_t count);
    int read_nonblocking(void *dest, uint32_t count);

    int write(void *dest, uint32_t count);

private:
    Channel(Channel const&);
    Channel & operator == (Channel const&);

    ChannelPrivate* d;
};

}
#endif // SSH_CHANNEL_H
