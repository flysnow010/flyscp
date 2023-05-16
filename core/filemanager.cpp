#include "filemanager.h"
#include <windows.h>
#include <oleidl.h>

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#define NTDDI_VERSION  NTDDI_WIN7 //WIN7
#endif
#include <Shlobj.h>
#include <knownfolders.h>
#include <QDebug>
#include <QFileInfo>
#include <QFileIconProvider>
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

void FileManager::Property(QStringList const& fileNames)
{
    IShellFolder* psfDesktop;
    HRESULT hr = SHGetDesktopFolder(&psfDesktop);
    if(FAILED(hr))
        return;

    LPITEMIDLIST *pidlDrives = (LPITEMIDLIST *)malloc(sizeof(LPITEMIDLIST)*fileNames.size());
    bool isFailed = false;
    for(int i = 0; i < fileNames.size(); i++)
    {
        QString fileName = fileNames[i].split("/").join("\\");
        hr = psfDesktop->ParseDisplayName(0, 0, (LPWSTR)(fileName.toStdWString().c_str()),
                                     0, (LPITEMIDLIST*)&pidlDrives[i], 0);
        if(FAILED(hr))
        {
            isFailed = true;
            break;
        }
    }

    if(!isFailed)
    {
        IDataObject* pData;
        hr = psfDesktop->GetUIObjectOf(0, fileNames.size(), (PCUITEMID_CHILD_ARRAY)pidlDrives,
                                       IID_IDataObject, 0, (void **)&pData);
        if(SUCCEEDED(hr))
        {
            SHMultiFileProperties(pData, 0);
            pData->Release();
        }
    }
    psfDesktop->Release();
    for(int i = 0; i < fileNames.size(); i++)
        ILFree(pidlDrives[i]);
    free(pidlDrives);
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
    ShellExecute(0, L"open", fileName.toStdWString().c_str(), 0,
                 QFileInfo(fileName).path().toStdWString().c_str(), SW_SHOWNORMAL);
}

void FileManager::OpenByExplorer(QString const& fileName)
{
    QString filePath = fileName.split("/").join("\\");
    ShellExecute(0, L"open", L"explorer", filePath.toStdWString().c_str(), 0, SW_SHOWNORMAL);
}

QString LibDir::showPath() const
{
    return QString("\\\\%1\\").arg(caption);
}

QIcon LibDir::icon() const
{
    QFileIconProvider fip;
    return fip.icon(QFileInfo(filePath));
}

QList<LibDir> FileManager::libDirs()
{
     QList<LibDir> dirs;
    KNOWNFOLDERID folders[] = {
        FOLDERID_Desktop,
        FOLDERID_Downloads,
        FOLDERID_Documents,
        FOLDERID_Pictures,
        FOLDERID_Music,
        FOLDERID_Videos
    };
    for(unsigned int i = 0; i < sizeof(folders) / sizeof(folders[0]); i++)
    {
        LPWSTR pszName;
        IShellItem *psi;
        LibDir dir;
        HRESULT hr = SHGetKnownFolderItem(folders[i], KF_FLAG_DEFAULT, 0,  IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            hr = psi->GetDisplayName(SIGDN_NORMALDISPLAY, &pszName);
            if (SUCCEEDED(hr))
                dir.caption = QString::fromStdWString(pszName);
            hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszName);
            if (SUCCEEDED(hr))
                dir.filePath = QString::fromStdWString(pszName);
            dirs <<  dir;
            psi->Release();
        }
    }
    return dirs;
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
