#ifndef SSH_FILEINFO_H
#define SSH_FILEINFO_H
#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace ssh {
class FileInfoPrivate;
class FileInfo
{
public:
    enum PermissionMask {
        User_Read   = (0x1 << 8),
        User_Write  = (0x1 << 7),
        User_Exe    = (0x1 << 6),
        Group_Read  = (0x1 << 5),
        Group_Write = (0x1 << 4),
        Group_Exe   = (0x1 << 3),
        Other_Read  = (0x1 << 2),
        Other_Write = (0x1 << 1),
        Other_Exe   = (0x1 << 0)
    };
    ~FileInfo();

    bool isNull() const;
    bool isDir() const;
    bool isFile() const;
    bool isSymLink() const;
    bool isParent() const;

    const char* name() const;
    std::string const& basename() const;
    std::string const& suffix() const;
    const char* owner() const;
    const char* group() const;
    const char* longname() const;

    uint32_t uid() const;
    uint32_t gid() const;
    uint32_t permissions() const;
    uint32_t time() const;

    uint64_t size() const;
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
    FileInfoPtr(FileInfo *fileInfo = 0)
        : std::shared_ptr<FileInfo>(fileInfo)
    {}
};

class FileInfos : public std::vector<FileInfoPtr>
{

};
}
#endif // SSH_FILEINFO_H
