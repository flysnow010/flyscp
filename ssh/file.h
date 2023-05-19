#ifndef SSH_FILE_H
#define SSH_FILE_H
#include <memory>

#include <cstdint>
#include <sys/types.h>

namespace ssh {
class FilePrivate;
class SFtp;
class Session;
class File
{
public:
    File(SFtp const& sftp);
    File(Session const& session);

    ~File();

    typedef std::shared_ptr<File> Ptr;

    void set_filesize(uint64_t filesize);
    bool open(const char *filename, int accesstype, mode_t mode);
    bool close();

    void set_noblocking(bool enable);
    ssize_t read(void *buf, size_t count);
    ssize_t write(const void *buf, size_t count);
    int seek(uint32_t new_offset);
    int seek(uint64_t new_offset);
    uint64_t tell();
    void rewind();
private:
    File(File const&);
    File & operator == (File const&);

    FilePrivate* d;
};
}

#endif // SSH_FILE_H
