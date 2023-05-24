#ifndef FILENAME_H
#define FILENAME_H
#include <QString>
#include <QList>

struct FileName
{
    QString src;
    QString dst;
};

struct FileInfo
{
    FileInfo(QString const& f, bool dir)
        : fileName(f)
        , isDir(dir)
    {}

    QString fileName;
    bool isDir = false;
};
typedef QList<FileInfo> FileInfos;

class FileNames : public QList<FileName>
{
public:
    static FileNames GetFileNames(QString const& fileName, QString const& filePath);
    static FileNames GetFileNames(QStringList const& fileNames, QString const& filePath);
    static void MakeFileNames(FileNames const& fileNames, FileNames & newFileNames);
    static void MakeFileNames(QStringList const& fileNames, QStringList & newFileNames);
    static void FindFilenames(FileName const& fileName, FileNames &fileNames);
    static void FindFilenames(QString const& fileName, QStringList &fileNames);
};
#endif
