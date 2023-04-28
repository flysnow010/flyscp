#ifndef SSH_FILEINFO_H
#define SSH_FILEINFO_H
#include <cstdint>
#include <list>
#include <memory>

namespace ssh {
class FileInfoPrivate;
class FileInfo
{
public:
    ~FileInfo();

    bool isNull() const;
    const char* name();
    const char* longname();
    uint64_t size();
private:
    friend class Dir;
    FileInfo();
    FileInfo(FileInfo const&);
    FileInfo & operator == (FileInfo const&);

    FileInfoPrivate* d;
};

class FileInfoPtr : public std::shared_ptr<FileInfo>
{
public:
    FileInfoPtr(FileInfo *fileInfo)
        : std::shared_ptr<FileInfo>(fileInfo)
    {}
};

class FileInfos : public std::list<FileInfoPtr>
{

};
}
#endif // SSH_FILEINFO_H
