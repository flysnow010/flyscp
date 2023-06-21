#include "winshell.h"
#include <windows.h>
#include <oleidl.h>

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#define NTDDI_VERSION  NTDDI_WIN7 //WIN7
#endif
#include <Shlobj.h>
#include <knownfolders.h>
#include <shlwapi.h> //for StrRetToStr
#include "shellitem.h"
#include <QString>
#include <QMimeData>
#include <QStringList>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QtWin>
#include <QDebug>

struct ShellHelper
{
    ShellHelper()
        : pDesktop(0)
        , pidlDrives(0)
        , pDataObject(0)
        , size(0)
    {
        SHGetDesktopFolder(&pDesktop);
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
        HRESULT hr;

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
        mineData->setData("text/uri-list", text.toUtf8());
        mineData->setData("FileName", GetBlob(GetClipboardFormat(CFSTR_FILENAMEA)));
        mineData->setData("FileNameW", GetBlob(GetClipboardFormat(CFSTR_FILENAMEW)));
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
}

void WinShell::OpenByExplorer(QString const& fileName)
{
    QString filePath = fileName.split("/").join("\\");
    ShellExecute(0, L"open", L"explorer",
                 filePath.toStdWString().c_str(), 0, SW_SHOWNORMAL);
}

void WinShell::Exec(QString const& appName, QStringList const& params)
{
    Exec(appName, params.join(" "));
}

void WinShell::Exec(QString const& appName, QString const& params)
{
    SHELLEXECUTEINFO shellExecInfo;
    memset(&shellExecInfo, 0, sizeof(shellExecInfo));
    std::wstring lpFile = appName.toStdWString();
    std::wstring lpParameters = params.toStdWString();
    std::wstring lpDirectory = QFileInfo(appName).path().toStdWString();
    shellExecInfo.cbSize = sizeof(shellExecInfo);
    shellExecInfo.lpFile = lpFile.c_str();
    shellExecInfo.lpParameters = lpParameters.c_str();
    shellExecInfo.lpDirectory = lpDirectory.c_str();
    shellExecInfo.lpVerb = L"open";
    shellExecInfo.nShow = SW_SHOWNORMAL;
    shellExecInfo.fMask = SEE_MASK_DEFAULT;

    ShellExecuteEx(&shellExecInfo);
}

bool WinShell::CreateShortcut(QString const& linkFilePath,
                              QString const& targetFilePath)
{
    IShellLink     *pShellLink;
    IPersistFile   *pPersistFile;

    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL,
    CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink);
    if(FAILED(hr))
        return false;

    hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
    if(FAILED(hr))
    {
         pShellLink->Release();
         return false;
    }

    pShellLink->SetPath(targetFilePath.toStdWString().c_str());
    pShellLink->SetWorkingDirectory(QFileInfo(targetFilePath).path().toStdWString().c_str());
    hr = pPersistFile->Save(linkFilePath.toStdWString().c_str(), TRUE);

    pPersistFile->Release();
    pShellLink->Release();

    return SUCCEEDED(hr);
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

ShellMenuItems WinShell::computerShellItems()
{
    LPITEMIDLIST pidl;
    HRESULT hr = SHGetSpecialFolderLocation(0, CSIDL_DRIVES, &pidl);
    ShellMenuItems items;
    if(SUCCEEDED(hr))
    {
        LPSHELLFOLDER  pDesktop = NULL;
        if(FAILED(SHGetDesktopFolder(&pDesktop)))
            return items;

        ShellMenuItem menutItem;
        pDesktop->AddRef();
        menutItem.pItem = ShellItem::Ptr(new ShellItem());
        menutItem.pItem->pidlRel = pidl;
        menutItem.pItem->pidlFQ = ShellItem::clone(pidl);
        menutItem.pItem->pParentFolder = pDesktop;

        shellSubMenuItems(menutItem, items, true);
        pDesktop->Release();
    }
    return items;
}

ShellMenuItems WinShell::shellMenuItems()
{
    LPITEMIDLIST pidl;
    HRESULT hr = SHGetSpecialFolderLocation(0, CSIDL_DESKTOP, &pidl);
    ShellMenuItems items;
    if(SUCCEEDED(hr))
    {
        ShellMenuItem menutItem;
        menutItem.pItem = ShellItem::Ptr(new ShellItem());
        menutItem.pItem->pidlRel = pidl;
        menutItem.pItem->pidlFQ = ShellItem::clone(pidl);

        shellSubMenuItems(menutItem, items, true);
    }
    return items;
}
static QString strToString(LPITEMIDLIST pidl, STRRET *str)
{
    LPTSTR pszText;
    QString text;
    HRESULT hr = StrRetToStr(str, pidl, &pszText);
    if (SUCCEEDED(hr))
    {
        text = QString::fromStdWString(pszText);
        CoTaskMemFree(pszText);
    }
    return text;
}
void WinShell::shellSubMenuItems(ShellMenuItem const& item,
                                 ShellMenuItems & menuItems,
                                 bool isOnlyDir)
{
    LPSHELLFOLDER  pParentFolder = NULL;
    HRESULT        hr;

    if(!item.pItem->pParentFolder)
        hr = SHGetDesktopFolder(&pParentFolder);
    else
        hr = item.pItem->pParentFolder->BindToObject(item.pItem->pidlRel, 0, IID_IShellFolder, (LPVOID*)&pParentFolder);

    if(FAILED(hr))
        return;

    LPENUMIDLIST   pEnum;
    SHCONTF grfFlags = SHCONTF_FOLDERS;
    if(!isOnlyDir)
        grfFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN;
    hr = pParentFolder->EnumObjects(NULL, grfFlags, &pEnum);
    if(SUCCEEDED(hr))
    {
        LPITEMIDLIST   pidlTemp;
        DWORD          dwFetched = 1;
        STRRET str;
        while(S_OK == (pEnum->Next(1, &pidlTemp, &dwFetched)) && dwFetched)
        {
            ShellMenuItem menutItem;
            pParentFolder->GetDisplayNameOf(pidlTemp, SHGDN_FORPARSING, &str);
            menutItem.filePath = strToString(pidlTemp, &str);
            menutItem.pItem = ShellItem::Ptr(new ShellItem());
            menutItem.pItem->pParentFolder = pParentFolder;
            menutItem.pItem->pidlRel = pidlTemp;
            menutItem.pItem->pidlFQ = ShellItem::concat(item.pItem->pidlFQ, pidlTemp);
            pParentFolder->AddRef();

            SHFILEINFO     sfi;
            if(SHGetFileInfo((LPCTSTR)menutItem.pItem->pidlFQ, 0, &sfi, sizeof(sfi),
                             SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ICON | SHGFI_SMALLICON))
            {
                menutItem.caption = QString::fromStdWString(sfi.szDisplayName);
                menutItem.icon = QIcon(QtWin::fromHICON(sfi.hIcon));
            }
            menuItems << menutItem;
            dwFetched = 0;
        }
        pEnum->Release();
    }
    pParentFolder->Release();
    std::sort(menuItems.begin(), menuItems.end());
}
