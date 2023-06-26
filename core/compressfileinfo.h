#ifndef COMPRESSFILEINFO_H
#define COMPRESSFILEINFO_H
#include <QStringList>
#include <QList>
#include <QStack>
#include <memory>

class CompressFileInfo
{
public:
    typedef std::shared_ptr<CompressFileInfo> Ptr;

    QString path() const { return path_; }
    QString fileName() const { return fileName_; }
    QString baseName() const { return baseName_; }
    QString suffix() const { return suffix_; }
    bool isDir() const { return isDir_; }
    bool isFile() const { return !isDir_; }
    QString timeText() const { return timeText_;  }
    quint32 time() const { return time_;  }
    quint64 size() const { return size_;  }
    quint64 compressedSize() const { return compressedSize_;  }
private:
    CompressFileInfo(){}
    QString path_;
    QString fileName_;
    QString baseName_;
    QString suffix_;
    bool isDir_ = false;
    QString timeText_;
    quint32 time_ = 0;
    quint64 size_ = 0;
    quint64 compressedSize_ = 0;
    friend class CompressFile;
};

typedef  QList<CompressFileInfo::Ptr> CompressFileInfos;

class CompressFile
{
public:
    enum SortFlag { Name        = 0x00,
                    Time        = 0x01,
                    Size        = 0x02,
                    Type        = 0x03,
                    Property    = 0x04,
                    Unsorted    = 0x07,
                    SortByMask  = 0x07,

                    DirsFirst   = 0x08,
                    Reversed    = 0x10,
                    IgnoreCase  = 0x20,
                    DirsLast    = 0x40,
                    LocaleAware = 0x80,
                    NoSort = -1
    };
    CompressFile();

    void setFileName(QString const& fileName, QStringList const& fileInfoLines);

    bool cd(QString const& dir);
    CompressFileInfos fileInfoList(SortFlag sortFlag = NoSort);
private:
    bool fineFileInfo(CompressFileInfos const& fileInfos, QString const& path);
    QString getSubDir(QString const& path, QString const& parent);
    void sort(CompressFileInfos &fileInfos, SortFlag sortFlag);
private:
    QString fileName_;
    CompressFileInfos fileInfos_;
    QStack<QString> dirs;
    QString currentDir;
};

#endif // COMPRESSFILEINFO_H
