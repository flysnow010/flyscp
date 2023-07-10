#include "filename.h"
#include <QFileInfo>
#include <QDir>

FileNames FileNames::GetFileNames(QString const& fileName,
                                  QString const& filePath)
{
    return GetFileNames(QStringList() << fileName, filePath);
}

FileNames FileNames::GetFileNames(QStringList const& fileNames,
                                  QString const& filePath)
{
    FileNames  newFileNames;
    QDir dir(filePath);
    for(int i = 0; i < fileNames.size(); i++)
    {
        if(fileNames[i].isEmpty())
            continue;

        QFileInfo fileInfo(fileNames[i]);

        FileName fileName;
        fileName.src = fileInfo.filePath();
        fileName.dst = dir.filePath(fileInfo.fileName());
        if(fileName.src == fileName.dst)
        {
            QString filename ;
            if(fileInfo.isDir())
                filename = QString("%1_copy").arg(fileInfo.completeBaseName());
            else
                filename = QString("%1_copy.%2").arg(fileInfo.completeBaseName(),
                                                     fileInfo.suffix());
            fileName.dst = dir.filePath(filename);
        }
        newFileNames << fileName;
    }
    return newFileNames;
}

void FileNames::MakeFileNames(FileNames const& fileNames,
                              FileNames & newFileNames)
{
    for(int i = 0; i < fileNames.size(); i++)
    {
        QFileInfo fileInfo(fileNames[i].src);
        if(fileInfo.isDir())
            FindFilenames(fileNames[i], newFileNames);
        else
            newFileNames << fileNames[i];
    }
}

void FileNames::MakeFileNames(QStringList const& fileNames,
                              QStringList & newFileNames)
{
    for(int i = 0; i < fileNames.size(); i++)
    {
        QFileInfo fileInfo(fileNames[i]);
        if(fileInfo.isDir() && !fileInfo.isSymLink())
            FindFilenames(fileNames[i], newFileNames);
        newFileNames << fileNames[i];
    }
}

void FileNames::FindFilenames(FileName const& fileName,
                              FileNames &fileNames)
{
    QDir srcDir = QDir(fileName.src);
    QDir dstDir = QDir(fileName.dst);
    QFileInfoList fileInfoList = srcDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot,
                                                      QDir::DirsFirst | QDir::Name);
    for(int i = 0; i < fileInfoList.size(); i++)
    {
        FileName f;
        f.src = fileInfoList[i].filePath();
        f.dst = dstDir.filePath(fileInfoList[i].fileName());

        if(fileInfoList[i].isDir())
            FindFilenames(f, fileNames);
        else
            fileNames << f;
    }
}

void FileNames::FindFilenames(QString const& fileName,
                              QStringList &fileNames)
{
    QDir dir = QDir(fileName);

    QFileInfoList fileInfoList = dir.entryInfoList(QDir::AllEntries
                                                   | QDir::NoDotAndDotDot
                                                   | QDir::Hidden,
                                                   QDir::DirsFirst
                                                   | QDir::Name);
    for(int i = 0; i < fileInfoList.size(); i++)
    {
        if(fileInfoList[i].isDir())
            FindFilenames(fileInfoList[i].filePath(), fileNames);
        fileNames << fileInfoList[i].filePath();
    }
}

void FileNames::MakeFileNamesAsParams(QStringList & fileNames)
{
    for(int i = 0; i < fileNames.size(); i++)
    {
        if(fileNames[i].contains(QChar(' ')))
            fileNames[i] = QString("%1%2%1").arg(QChar('"'), fileNames[i]);
    }
}

void FileNames::MakeFileNameAsParams(QString & fileName)
{
    if(fileName.contains(QChar(' ')))
        fileName = QString("%1%2%1").arg(QChar('"'), fileName);
}

