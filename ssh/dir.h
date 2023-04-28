#ifndef SSH_DIR_H
#define SSH_DIR_H
#include <memory>

namespace ssh {
class DirPrivate;
class SFtp;
class FileInfoPtr;
class FileInfos;
class Dir
{
public:
    Dir(SFtp const& sftp, const char* path);
    ~Dir();

    const char* dirName();
    FileInfos fileInfoList();
    FileInfoPtr fileInfo(const char* path);
private:
    Dir(Dir const&);
    Dir & operator == (Dir const&);

    DirPrivate* d;
};

class DirPtr : public std::shared_ptr<Dir>
{
public:
    DirPtr(Dir *d)
        : std::shared_ptr<Dir>(d)
    {}
};

}

#endif // SSH_DIR_H
