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

    void reset();

    bool open();
    bool close();
    bool send_eof();

    bool run_shell(int cols, int rows);
    bool is_open();
    bool is_eof();

    bool shell_size(int cols, int rows);

    bool exec(const char* command);

    int poll();
    int poll(int timeout);
    void set_blocking(bool  enable);
    int read(void *dest, uint32_t count);
    int read_nonblocking(void *dest, uint32_t count);

    int write(void *dest, uint32_t count);

private:
    friend class SFtp;
    Channel(Channel const&);
    Channel & operator == (Channel const&);

    ChannelPrivate* d;
};

}
#endif // SSH_CHANNEL_H
