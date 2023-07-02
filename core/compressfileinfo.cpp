#include "compressfileinfo.h"
#include "fileuncompresser.h"
#include "filecompresser.h"
#include "util/utils.h"

#include <QTextStream>
#include <QFileInfo>
#include <QFile>
#include <QDateTime>
#include <QDebug>

#include <algorithm>

#define WINDOWS_SEP "\\"

CompressFile::CompressFile()
{
}

void CompressFile::setFileName(QString const& fileName)
{
    fileName_ = Utils::toWindowsPath(fileName);
    refresh(false);
}

QString CompressFile::fileName() const
{
    return fileName_;
}

bool CompressFile::setDir(QString const& dir)
{
    if(dir == fileName_)
    {
        refresh(false);
        return true;
    }
    if(fileName_.isEmpty())
        fileName_ = getCompressFile(dir);

    if(fileName_.isEmpty())
        return false;

    QString subDir = getSubDir(dir, fileName_, true);
    if(subDir.isEmpty())
        return false;

    refresh(false);

    QStringList dirs = subDir.split(WINDOWS_SEP);
    foreach(auto d, dirs)
        cd(d);

    return true;
}

QString CompressFile::dir() const
{
    if(currentDir_.isEmpty())
        return fileName_;
    return QString("%1\\%2").arg(fileName_, currentDir_);
}

bool CompressFile::cd(QString const& dir)
{
    if(currentDir_.isEmpty() && dir == "..")
        return false;

    if(dir == "..")
        dirs.pop();
    else
        dirs.push(dir);

    QStringList dirList;

    foreach(auto d, dirs)
        dirList << d;
    currentDir_ = dirList.join(WINDOWS_SEP);
    return true;
}

CompressFileInfos CompressFile::fileInfoList(SortFlag sortFlag)
{
    CompressFileInfos fileInfos;
    foreach(auto fileInfo, fileInfos_)
    {
        if(fileInfo->isFile())
        {
            if(fileInfo->path() == currentDir_)
                fileInfos << fileInfo;
        }
        else if(fileInfo->isDir())
        {
            QString subDir = getSubDir(fileInfo->path_, currentDir_, true);
            if(subDir.isEmpty())
                continue;

            QStringList paths = subDir.split(WINDOWS_SEP);
            if(!paths.isEmpty())
            {
                if(paths.size() == 1)
                {
                    if(!findFileInfo(fileInfos, paths[0]))
                    {
                        CompressFileInfo::Ptr newFileInfo(new CompressFileInfo());
                        newFileInfo->parent_ = currentDir_;
                        newFileInfo->path_ = paths[0];
                        newFileInfo->timeText_ = fileInfo->timeText_;
                        newFileInfo->attributes_ = QString("D.....");
                        newFileInfo->isDir_ = true;
                        fileInfos << newFileInfo;
                    }
                }
                else if(currentDir_.isEmpty())
                {
                    if(!findFileInfo(fileInfos, paths[0]))
                    {
                        CompressFileInfo::Ptr newFileInfo(new CompressFileInfo());
                        newFileInfo->path_ = paths[0];
                        newFileInfo->timeText_ = fileInfo->timeText_;
                        newFileInfo->attributes_ = QString("D.....");
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

bool CompressFile::mkdir(QString const& dir)
{
    QString filePath;
    if(currentDir_.isEmpty())
    {
        QDir tempDir = Utils::tempDir();
        tempDir.mkdir(dir);
        tempDir.cd(dir);
        filePath = Utils::toWindowsPath(tempDir.path()) + WINDOWS_SEP;
    }
    else
    {
        QDir tempDir = Utils::tempDir();
        tempDir.mkpath(QString("%1/%2").arg(currentDir_, dir));
        int index = currentDir_.indexOf(WINDOWS_SEP);
        if(index < 0)
            filePath =  Utils::toWindowsPath(QString("%1/%2/")
                                             .arg(tempDir.path(), currentDir_));
        else
            filePath =  Utils::toWindowsPath(QString("%1/%2/")
                                             .arg(tempDir.path(), currentDir_.left(index)));
    }
    if(!FileCompresser().update(QStringList() << filePath, fileName_))
        return false;

    QDir tempDir = Utils::tempDir();
    if(currentDir_.isEmpty())
        tempDir.rmdir(dir);
    else
        tempDir.rmpath(QString("%1\\%2").arg(currentDir_, dir));

    //qDebug() << QString("%1\\%2").arg(currentDir_, dir); ???
    return true;
}

bool CompressFile::rm(QStringList const& fileNames)
{
    return FileUncompresser().remove(fileName_, fileNames);
}

bool CompressFile::add(QStringList const& fileNames)
{
    QStringList renameFileNames;

    if(FileCompresser().update(fileNames, fileName_))
    {
        if(!currentDir_.isEmpty())
            rename(fileNames, QString());
        return true;
    }
    return false;
}

bool CompressFile::rename(QString const& oldFileName,
                          QString const& newFileName)
{
    return FileUncompresser().rename(fileName_,
                                     filePath(oldFileName),
                                     filePath(newFileName));
}

bool CompressFile::rename(QStringList const& fileNames,
                          QString const& subPath)
{
    if(currentDir_.isEmpty())
        return false;

    QStringList renameFileNames;
    foreach(auto fileName, fileNames)
    {
        QString oldFileName = QFileInfo(fileName).fileName();
        QString newFileName;
        if(subPath.isEmpty())
            newFileName  = QString("%1\\%2")
                    .arg(currentDir_, oldFileName);
        else
            newFileName  = QString("%1\\%2\\%3")
                    .arg(currentDir_, subPath, oldFileName);
        renameFileNames << oldFileName << newFileName;
    }

    return FileUncompresser().rename(fileName_, renameFileNames);
}

bool CompressFile::extract(QString const& targetPath,
                           QStringList const& fileNames,
                           bool isWithPath)
{
    return FileUncompresser().extract(fileName_,
                                      targetPath,
                                      fileNames,
                                      isWithPath);
}

QString CompressFile::filePath(QString const& fileName) const
{
    if(currentDir_.isEmpty())
        return fileName;
    return QString("%1\\%2").arg(currentDir_, fileName);
}

void CompressFile::refresh(bool isCurrent)
{
    QStringList fileInfoLines = FileUncompresser().listFileInfo(fileName_);

    if(!isCurrent)
    {
        currentDir_ = "";
        dirs.clear();
    }
    fileInfos_.clear();

    QString dateTime("2023-06-24 15:49:00");
    QDateTime start(QDate(1970, 1,1), QTime(0, 0, 0));

    foreach(auto line, fileInfoLines)
    {
        QTextStream stream(&line);
        CompressFileInfo::Ptr info(new CompressFileInfo());

        info->timeText_ = stream.read(dateTime.size());
        stream.skipWhiteSpace();
        stream >> info->attributes_ >> info->size_ >> info->compressedSize_;
        info->isDir_ = info->attributes_.startsWith("D");
        info->time_ = start.secsTo(QDateTime::fromString(info->timeText_,
                                                         "yyyy-MM-dd HH:mm:ss"));
        stream.skipWhiteSpace();

        QString name = stream.readAll();
        QFileInfo fileInfo(name);

        if(info->isDir())
            info->path_ = name;
        else
        {
            info->path_ = Utils::toWindowsPath(fileInfo.path());
            if(info->path_ == ".")
                info->path_ = "";
            info->fileName_ = fileInfo.fileName();
            info->baseName_ = fileInfo.completeBaseName();
            info->suffix_ = fileInfo.suffix();
        }
        fileInfos_ << info;
    }
}

struct CompressFileInfoCompare
{
    CompressFileInfoCompare(CompressFile::SortFlag f)
        : flag(f)
    {}

    bool operator() (CompressFileInfo::Ptr const& l,
                     CompressFileInfo::Ptr const& r)
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

    bool compare(CompressFileInfo::Ptr const& l,
                 CompressFileInfo::Ptr const& r)
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

void CompressFile::sort(CompressFileInfos &fileInfos,
                        SortFlag sortFlag)
{
    std::sort(fileInfos.begin(), fileInfos.end(),
              CompressFileInfoCompare(sortFlag));
}

QString CompressFile::getCompressFile(QString const& filePath)
{
    QString path = filePath;
    while(true)
    {
        if(QFile::exists(path))
            return path;

        int index = path.lastIndexOf(WINDOWS_SEP);
        if(index < 0)
            break;
        path = path.left(index);
    }

    return QString();
}

QString CompressFile::getSubDir(QString const& path,
                                QString const& parent,
                                bool isWindowSep)
{
    QString sep = isWindowSep ? QString("\\") : QString("/");

    if(parent.isEmpty())
        return path;

    int index = path.indexOf(parent + sep);
    if(index >= 0)
        return path.mid(index + 1 + parent.size());

    return QString();
}

bool CompressFile::findFileInfo(CompressFileInfos const& fileInfos,
                                QString const& path)
{
    foreach(auto fileInfo, fileInfos)
    {
        if(fileInfo->path() == path)
            return true;
    }
    return false;
}

