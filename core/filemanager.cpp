#include "filemanager.h"
#include <windows.h>
#include <oleidl.h>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

static DWORD ProgressCallback(
    LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER /*StreamSize*/,
    LARGE_INTEGER /*StreamBytesTransferred*/,
    DWORD /*dwStreamNumber*/,
    DWORD /*dwCallbackReason*/,
    HANDLE /*hSourceFile*/,
    HANDLE /*hDestinationFile*/,
    LPVOID lpData
)
{
    return FileManager::Progress(lpData, TotalFileSize.QuadPart, TotalBytesTransferred.QuadPart);
}

FileManager::FileManager(QObject *parent)
    : QObject(parent)
    , signal_(false)
{

}

quint32 FileManager::Progress(void *pData, qint64 TotalFileSize, qint64 TotalBytesTransferred)
{
    FileManager* fileManger = (FileManager *)pData;
    return fileManger->onProgress(TotalFileSize, TotalBytesTransferred);
}

quint32 FileManager::onProgress(qint64 TotalFileSize, qint64 TotalBytesTransferred)
{
    emit fileProgress(TotalFileSize, TotalBytesTransferred);
    if(singled())
      return  PROGRESS_CANCEL;
    return PROGRESS_CONTINUE;
}

void FileManager::makeFileNames(FileNames const& fileNames, FileNames & newFileNames)
{
    for(int i = 0; i < fileNames.size(); i++)
    {
        QFileInfo fileInfo(fileNames[i].src);
        if(fileInfo.isDir())
            findFilenames(fileNames[i], newFileNames);
        else
            newFileNames << fileNames[i];
    }
}

void FileManager::makeFileNames(QStringList const& fileNames, QStringList & newFileNames)
{
    for(int i = 0; i < fileNames.size(); i++)
    {
        QFileInfo fileInfo(fileNames[i]);
        if(fileInfo.isDir())
            findFilenames(fileNames[i], newFileNames);
        newFileNames << fileNames[i];
    }
}

void FileManager::findFilenames(FileName const& fileName, FileNames &fileNames)
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
            findFilenames(f, fileNames);
        else
            fileNames << f;
    }
}

void FileManager::findFilenames(QString const& fileName, QStringList &fileNames)
{
    QDir dir = QDir(fileName);

    QFileInfoList fileInfoList = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot,
                                                   QDir::DirsFirst | QDir::Name);
    for(int i = 0; i < fileInfoList.size(); i++)
    {
        if(fileInfoList[i].isDir())
            findFilenames(fileInfoList[i].filePath(), fileNames);
        fileNames << fileInfoList[i].filePath();
    }
}
/*
edit 用编辑器打开 lpFile 指定的文档，如果 lpFile 不是文档，则会失败
explore 浏览 lpFile 指定的文件夹
find 搜索 lpDirectory 指定的目录
open 打开 lpFile 文件，lpFile 可以是文件或文件夹
openas
print 打印 lpFile，如果 lpFile 不是文档，则函数失败
properties 显示属性
runas 请求以管理员权限运行，比如以管理员权限运行某个exe
*/
void FileManager::Property(QString const& fileName)
{
    SHELLEXECUTEINFO shellExecInfo;
    memset(&shellExecInfo, 0, sizeof(shellExecInfo));

    shellExecInfo.cbSize = sizeof(shellExecInfo);
    shellExecInfo.lpFile = fileName.toStdWString().c_str();
    shellExecInfo.lpVerb = L"properties";
    shellExecInfo.nShow = SW_SHOWNORMAL;
    shellExecInfo.fMask = SEE_MASK_INVOKEIDLIST;

    ShellExecuteEx(&shellExecInfo);
    WaitForSingleObject(shellExecInfo.hProcess, INFINITE);
}

void FileManager::OpenWith(QString const& fileName)
{
    SHELLEXECUTEINFO shellExecInfo;
    memset(&shellExecInfo, 0, sizeof(shellExecInfo));

    shellExecInfo.cbSize = sizeof(shellExecInfo);
    shellExecInfo.lpFile = fileName.toStdWString().c_str();
    shellExecInfo.lpVerb = L"openas";
    shellExecInfo.nShow = SW_SHOWNORMAL;
    shellExecInfo.fMask = SEE_MASK_INVOKEIDLIST;

    ShellExecuteEx(&shellExecInfo);
    WaitForSingleObject(shellExecInfo.hProcess, INFINITE);
}

void FileManager::Open(QString const& fileName)
{
    ShellExecute(0, L"open", fileName.toStdWString().c_str(), 0, 0, SW_SHOWNORMAL);
}

void FileManager::OpenByExplorer(QString const& fileName)
{
    QString filePath = fileName.split("/").join("\\");
    ShellExecute(0, L"open", L"explorer", filePath.toStdWString().c_str(), 0, SW_SHOWNORMAL);
}

void FileManager::delereFiles(QStringList const& fileNames)
{
    QStringList newFileNames;
    makeFileNames(fileNames, newFileNames);
    for(int i = 0; i < newFileNames.size(); i++)
    {
        if(singled())
            break;

        emit totalProgress(newFileNames[i], QString(), newFileNames.size(), i);
        QFileInfo fileInfo(newFileNames[i]);
        if(fileInfo.isDir())
            RemoveDirectory(newFileNames[i].toStdWString().c_str());
        else
            DeleteFile(newFileNames[i].toStdWString().c_str());
        emit totalProgress(newFileNames[i], QString(), newFileNames.size(), i + 1);
    }
    emit finished();
}

void FileManager::copyFiles(FileNames const& fileNames)
{
    FileNames newFileNames;
    makeFileNames(fileNames, newFileNames);
    for(int i = 0; i < newFileNames.size(); i++)
    {
        if(singled())
            break;
        emit totalProgress(newFileNames[i].src, newFileNames[i].dst, newFileNames.size(), i);
        QDir dir;
        dir.mkpath(QFileInfo(newFileNames[i].dst).path());
        CopyFileEx(newFileNames[i].src.toStdWString().c_str(),
                   newFileNames[i].dst.toStdWString().c_str(),
                   ProgressCallback, this, 0,   COPY_FILE_OPEN_SOURCE_FOR_WRITE);
        emit totalProgress(newFileNames[i].src, newFileNames[i].dst, newFileNames.size(), i + 1);
    }
    emit finished();
}

void FileManager::moveFiles(FileNames const& fileNames)
{
    FileNames newFileNames;
    makeFileNames(fileNames, newFileNames);
    for(int i = 0; i < newFileNames.size(); i++)
    {
        if(singled())
            break;
        emit totalProgress(newFileNames[i].src, newFileNames[i].dst, newFileNames.size(), i);
        MoveFileWithProgress(newFileNames[i].src.toStdWString().c_str(),
                             newFileNames[i].dst.toStdWString().c_str(),
                             ProgressCallback, this,
                             MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
        emit totalProgress(newFileNames[i].src, newFileNames[i].dst, newFileNames.size(), i + 1);
    }
    emit finished();
}

void FileManager::cancel()
{
    doSignal();
}
