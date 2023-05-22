#include "winshell.h"
#include <windows.h>
#include <oleidl.h>

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#define NTDDI_VERSION  NTDDI_WIN7 //WIN7
#endif
#include <Shlobj.h>
#include <knownfolders.h>

#include <QString>
#include <QMimeData>
#include <QStringList>
#include <QFileInfo>
#include <QFileIconProvider>

struct ShellHelper
{
    ShellHelper()
        : pDesktop(0)
        , pidlDrives(0)
        , pDataObject(0)
        , size(0)
    {
    }

    ~ShellHelper()
    {
        if(pDataObject)
            pDataObject->Release();
        if(pDesktop)
            pDesktop->Release();
        if(pidlDrives)
        {
            for(int i = 0; i < size; i++)
                ILFree(pidlDrives[i]);
        }
    }

    inline bool open(QStringList const& fileNames)
    {
        HRESULT hr = SHGetDesktopFolder(&pDesktop);
        if(FAILED(hr))
            return  false;

        pidlDrives = (LPITEMIDLIST *)malloc(sizeof(LPITEMIDLIST)*fileNames.size());
        size = fileNames.size();
        for(int i = 0; i < fileNames.size(); i++)
        {
            QString fileName = fileNames[i].split("/").join("\\");
            hr = pDesktop->ParseDisplayName(0, 0, (LPWSTR)(fileName.toStdWString().c_str()),
                                         0, (LPITEMIDLIST*)&pidlDrives[i], 0);
            if(FAILED(hr))
                return false;
        }
        hr = pDesktop->GetUIObjectOf(0, fileNames.size(), (PCUITEMID_CHILD_ARRAY)pidlDrives,
                                       IID_IDataObject, 0, (void **)&pDataObject);
        if(FAILED(hr))
            return false;
        return true;
    }

    inline void property(QStringList const& fileNames)
    {
        if(open(fileNames))
            SHMultiFileProperties(pDataObject, 0);
    }

    inline void copy(QStringList const& fileNames, bool isMove)
    {
        if(open(fileNames))
        {
            DWORD data = isMove ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
            SetBlob(GetClipboardFormat(CFSTR_PREFERREDDROPEFFECT), &data, sizeof(DWORD));
            OleSetClipboard(pDataObject);
        }
    }
    QMimeData* dropMimeData(QStringList const& fileNames)
    {
        if(!open(fileNames))
            return 0;
        QMimeData* mineData = new QMimeData();
        QString text = fileNames.join("\n");
        if(fileNames.size() > 1)
            text += "\n";
        mineData->setData("Shell IDList Array", GetBlob(GetClipboardFormat(CFSTR_SHELLIDLIST)));
        mineData->setText(text);
        mineData->setData("FileName", GetBlob(GetClipboardFormat(CFSTR_FILENAMEA)));
        mineData->setData("FileContents", GetBlob(GetClipboardFormat(CFSTR_FILECONTENTS)));
        mineData->setData("FileNameW", GetBlob(GetClipboardFormat(CFSTR_FILENAMEW)));
        mineData->setData("FileGroupDescriptorW", GetBlob(GetClipboardFormat(CFSTR_FILEDESCRIPTORW)));
        return mineData;
    }

    inline CLIPFORMAT GetClipboardFormat(PCWSTR pszForamt)
    {
        return (CLIPFORMAT)RegisterClipboardFormat(pszForamt);
    }

    HRESULT SetBlob(CLIPFORMAT cf, const void *pvBlob, UINT cbBlob)
    {
        void *pv = GlobalAlloc(GPTR, cbBlob);
        HRESULT hr = pv ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            CopyMemory(pv, pvBlob, cbBlob);

            FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
            STGMEDIUM medium = {};
            medium.tymed = TYMED_HGLOBAL;
            medium.hGlobal = pv;

            hr = pDataObject->SetData(&fmte, &medium, TRUE);
            if (FAILED(hr))
            {
                GlobalFree(pv);
            }
        }
        return hr;
    }

    QByteArray GetBlob(CLIPFORMAT cf)
    {
        FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM medium = {};
        QByteArray bytes;
        HRESULT hr = pDataObject->GetData(&fmte, &medium);
        if (SUCCEEDED(hr))
        {
            size_t size = GlobalSize(medium.hGlobal);
            bytes.resize(size);
            CopyMemory(bytes.data(), medium.hGlobal, size);
        }
        return bytes;
    }

    IShellFolder* pDesktop;
    LPITEMIDLIST* pidlDrives;
    IDataObject* pDataObject;
    int size;
};

QString WinLibDir::showPath() const
{
        return QString("\\\\%1\\").arg(caption);
}

QIcon   WinLibDir::icon() const
{
    QFileIconProvider fip;
    return fip.icon(QFileInfo(filePath));
}

WinShell::WinShell()
{
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
void WinShell::Property(QString const& fileName)
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

void WinShell::Property(QStringList const& fileNames)
{
    ShellHelper().property(fileNames);
}

void WinShell::Copy(QStringList const& fileNames, bool isMove)
{
    ShellHelper().copy(fileNames, isMove);
}

QMimeData* WinShell::dropMimeData(QStringList const& fileNames)
{
    return ShellHelper().dropMimeData(fileNames);
}

void WinShell::Open(QString const& fileName)
{
    ShellExecute(0, L"open", fileName.toStdWString().c_str(), 0,
                 QFileInfo(fileName).path().toStdWString().c_str(),
                 SW_SHOWNORMAL);
}

void WinShell::OpenWith(QString const& fileName)
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

void WinShell::OpenByExplorer(QString const& fileName)
{
    QString filePath = fileName.split("/").join("\\");
    ShellExecute(0, L"open", L"explorer",
                 filePath.toStdWString().c_str(), 0, SW_SHOWNORMAL);
}

QList<WinLibDir> WinShell::winLibDirs()
{
   QList<WinLibDir> dirs;
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
       WinLibDir dir;
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