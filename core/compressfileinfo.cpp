#include "compressfileinfo.h"
#include <QTextStream>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

#include <algorithm>

CompressFile::CompressFile()
{
}

void CompressFile::setFileName(QString const& fileName, QStringList const& fileInfoLines)
{
    fileName_ = fileName;
    currentDir = "";
    fileInfos_.clear();
    dirs.clear();
    QString dateTime("2023-06-24 15:49:00");
    QDateTime start(QDate(1970, 1,1), QTime(0, 0, 0));
    foreach(auto line, fileInfoLines)
    {
        QTextStream stream(&line);
        CompressFileInfo::Ptr info(new CompressFileInfo());
        QString type;
        QString name;
        info->timeText_ = stream.read(dateTime.size());
        stream.skipWhiteSpace();
        stream >> type >> info->size_ >> info->compressedSize_;
        info->isDir_ = type.startsWith("D");
        info->time_ = start.secsTo(QDateTime::fromString(info->timeText_, "yyyy-MM-dd HH:mm:ss"));
        stream.skipWhiteSpace();
        name = stream.readAll().split("\\").join("/");
        QFileInfo fileInfo(name);
        if(info->isDir())
            info->path_ = name;
        else
        {
            info->path_ = fileInfo.path();
            if(info->path_ == ".")
                info->path_ = "";
            info->fileName_ = fileInfo.fileName();
            info->baseName_ = fileInfo.completeBaseName();
            info->suffix_ = fileInfo.suffix();
        }
        fileInfos_ << info;
    }
}
bool CompressFile::cd(QString const& dir)
{
    if(currentDir.isEmpty() && dir == "..")
        return false;

    if(dir == "..")
        dirs.pop();
    else
        dirs.push(dir);
    QStringList dirList;
    foreach(auto d, dirs)
    {
        dirList << d;
    }
    currentDir = dirList.join("/");
    return true;
}

CompressFileInfos CompressFile::fileInfoList(SortFlag sortFlag)
{
    CompressFileInfos fileInfos;
    foreach(auto fileInfo, fileInfos_)
    {
        if(fileInfo->isFile())
        {
            if(fileInfo->path() == currentDir)
                fileInfos << fileInfo;
        }
        else if(fileInfo->isDir())
        {
            QString subDir = getSubDir(fileInfo->path_, currentDir);
            if(subDir.isEmpty())
                continue;
            QStringList paths = subDir.split("/");
            if(!paths.isEmpty())
            {
                if(paths.size() == 1)
                {
                    if(!fineFileInfo(fileInfos, paths[0]))
                    {
                        CompressFileInfo::Ptr newFileInfo(new CompressFileInfo());
                        newFileInfo->path_ = paths[0];
                        newFileInfo->timeText_ = fileInfo->timeText_;
                        newFileInfo->isDir_ = true;
                        fileInfos << newFileInfo;
                    }
                }
                else if(currentDir.isEmpty())
                {
                    if(!fineFileInfo(fileInfos, paths[0]))
                    {
                        CompressFileInfo::Ptr newFileInfo(new CompressFileInfo());
                        newFileInfo->path_ = paths[0];
                        newFileInfo->isDir_ = true;
                        fileInfos << newFileInfo;
                    }
                }
            }
        }
    }
    sort(fileInfos, sortFlag);
    CompressFileInfo::Ptr parent(new CompressFileInfo());
    parent->isDir_ = true;
    parent->path_ = "..";
    fileInfos.push_front(parent);
    return fileInfos;
}

struct CompressFileInfoCompare
{
    CompressFileInfoCompare(CompressFile::SortFlag f)
        : flag(f)
    {}

    bool operator() (CompressFileInfo::Ptr const& l, CompressFileInfo::Ptr const& r)
    {
        if(flag == CompressFile::NoSort)
            return false;

        if(l->isDir())
        {
            if(r->isFile())
            {
                if(flag & CompressFile::DirsFirst)
                    return true;
                return false;
            }
            else
            {
                if(flag & CompressFile::Reversed)
                    return compare(l, r);
                return compare(r, l);
            }
        }
        else
        {
            if(r->isDir())
            {
                if(flag & CompressFile::DirsFirst)
                    return false;
                return true;
            }
            else
            {
                if(flag & CompressFile::Reversed)
                    return compare(l, r);
                return compare(r, l);
            }
        }
    }

    bool compare(CompressFileInfo::Ptr const& l, CompressFileInfo::Ptr const& r)
    {
        uint32_t sort = flag & CompressFile::SortByMask;
        if(sort == CompressFile::Name)
            return l->baseName() < r->baseName();
        else if(sort == CompressFile::Time)
            return l->time() < r->time();
        else if(sort == CompressFile::Size)
            return l->size() < r->size();
        else if(sort == CompressFile::Type)
            return l->suffix() < r->suffix();
        else
            return false;
    }

    CompressFile::SortFlag flag;
};

void CompressFile::sort(CompressFileInfos &fileInfos, SortFlag sortFlag)
{
    std::sort(fileInfos.begin(), fileInfos.end(), CompressFileInfoCompare(sortFlag));
}

QString CompressFile::getSubDir(QString const& path, QString const& parent)
{
    if(parent.isEmpty())
        return path;
    int index = path.indexOf(parent + "/");
    if(index >= 0)
        return path.mid(index + 1 + parent.size());
    return QString();
}

bool CompressFile::fineFileInfo(CompressFileInfos const& fileInfos, QString const& path)
{
    foreach(auto fileInfo, fileInfos)
    {
        if(fileInfo->path() == path)
            return true;
    }
    return false;
}

