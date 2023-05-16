#ifndef SSH_SCP_H
#define SSH_SCP_H
#include <cstdint>
#include <sys/types.h>

namespace ssh {
class ScpPrivate;
class Session;

class Scp
{
public:
    Scp(Session const& session);
    ~Scp();
    enum Req { MewDir = 1, NewFile, Eof, EmdDir, Warning };

    bool open(const char* filepath, bool isRead);
    void close();

    //for read
    int pull_req();
    size_t get_size();
    const char* get_filename();
    int get_permissions();
    const char* get_warning();
    bool accept_req();
    bool deny_req(const char* reason);

    ssize_t read(void *buf, size_t count);

    //for write
    bool mkdir(const char* dirname);
    bool leave_dir();
    bool mkfile(const char* filename, size_t count);
    ssize_t write(const void *buf, size_t count);

private:
    Scp(Scp const&);
    Scp & operator == (Scp const&);

    ScpPrivate* d;
};

}

#endif // SSH_SCP_H
