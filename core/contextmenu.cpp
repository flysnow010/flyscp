#include "contextmenu.h"
#include "shellitem.h"
#include "util/utils.h"
#include <Shlobj.h>
#include <shlwapi.h>
#include <windows.h>

#include <QSettings>
#include <QDebug>
#include <QAxObject>
#include <QFileIconProvider>

#include <algorithm>

struct ContextMenuHelper
{
    static ContextMenuHelper* Instatnce()
    {
        static ContextMenuHelper helper;
        return &helper;
    }

    ~ContextMenuHelper()
    {
        close();
        if(pDesktop)
            pDesktop->Release();
    }

    inline bool open(QStringList const& fileNames, bool isOpenDataObject = true)
    {
        HRESULT hr;

        pidlDrives = (LPITEMIDLIST *)malloc(sizeof(LPITEMIDLIST)*fileNames.size());
        size = fileNames.size();
        for(int i = 0; i < fileNames.size(); i++)
        {
            QString fileName = Utils::toWindowsPath(fileNames[i]);
            hr = pDesktop->ParseDisplayName(0, 0, (LPWSTR)(fileName.toStdWString().c_str()),
                                         0, (LPITEMIDLIST*)&pidlDrives[i], 0);
            if(FAILED(hr))
                return false;
        }
        if(isOpenDataObject)
        {
            hr = pDesktop->GetUIObjectOf(0, fileNames.size(), (PCUITEMID_CHILD_ARRAY)pidlDrives,
                                       IID_IDataObject, 0, (void **)&pDataObject);
            if(FAILED(hr))
                return false;
        }
        return true;
    }


    inline void close()
    {
        if(pDataObject)
        {
            pDataObject->Release();
            pDataObject = 0;
        }
        if(pidlDrives)
        {
            for(int i = 0; i < size; i++)
                ILFree(pidlDrives[i]);
            free(pidlDrives);
            pidlDrives = 0;
        }
    }

    bool isRootPath(QString const& path)
    {
        //for dirver D: D:/
        //for virtual driver, the path's len is larger than 3
        return path.size() > 1 && path.size() <= 3;
    }
    void showContextMenu(QStringList const& fileNames, void* handle, int x, int y)
    {
        QString parentPath = Utils::toWindowsPath(QFileInfo(fileNames[0]).path());
        LPSHELLFOLDER pParentFolder = getParentFolder(parentPath, isRootPath(fileNames[0]));
        if(!pParentFolder)
            return;

        LPITEMIDLIST* pidlFiles = (LPITEMIDLIST *)malloc(sizeof(LPITEMIDLIST) * fileNames.size());
        {
            LPENUMIDLIST pEnum;
            HRESULT hr = pParentFolder->EnumObjects(0, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &pEnum);
            if(SUCCEEDED(hr))
            {
                LPITEMIDLIST pidl;
                STRRET str;
                int index = 0;
                while(pEnum->Next(1, &pidl, 0) == S_OK)
                {
                    pParentFolder->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &str);
                    QString fileName = Utils::toLinuxPath(strToString(pidl, &str));
                    if(!fileNames.contains(fileName))
                        CoTaskMemFree(pidl);
                    else
                    {
                        if(index < fileNames.size())
                            pidlFiles[index++] = pidl;
                    }
                }
            }
        }

        IContextMenu   *pcm;
        HRESULT hr = pParentFolder->GetUIObjectOf(0,
                                 fileNames.size(),
                                 (PCUITEMID_CHILD_ARRAY)pidlFiles,
                                 IID_IContextMenu,
                                 0,
                                 (LPVOID*)&pcm);
        if(SUCCEEDED(hr))
        {
            HMENU hPopup = CreatePopupMenu();
            if(hPopup)
            {
                hr = pcm->QueryContextMenu(hPopup, 0, 1, 0x7fff,  CMF_NORMAL);

                if(SUCCEEDED(hr))
                {
                    IContextMenu2  *pcm2;
                    pcm->QueryInterface(IID_IContextMenu2, (LPVOID*)&pcm2);
                    UINT  idCmd;

                    idCmd = TrackPopupMenu( hPopup,
                                            TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
                                            x,
                                            y,
                                            0,
                                            (HWND)handle,
                                            0);
                    if(pcm2)
                    {
                        pcm2->Release();
                        pcm2 = NULL;
                    }
                    if(idCmd && (idCmd != (UINT)-1))
                    {
                        CMINVOKECOMMANDINFO  cmi;
                        cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
                        cmi.fMask = 0;
                        cmi.hwnd = 0;
                        cmi.lpVerb = (LPCSTR)(INT_PTR)(idCmd - 1);
                        cmi.lpParameters = NULL;
                        cmi.lpDirectory = NULL;
                        cmi.nShow = SW_SHOWNORMAL;
                        cmi.dwHotKey = 0;
                        cmi.hIcon = NULL;
                        pcm->InvokeCommand(&cmi);
                    }
                }
                DestroyMenu(hPopup);
            }
        }
        for(int i = 0; i < fileNames.size(); i++)
            ILFree(pidlFiles[i]);
        free(pidlFiles);
        pParentFolder->Release();
    }

    LPSHELLFOLDER getParentFolder(QString const& filePath, bool isRoot = false)
    {
        LPSHELLFOLDER pDrives = getSpecialFolder(CSIDL_DRIVES);
        ShellItem::Ptr item;
        if(pDrives)
        {
            LPENUMIDLIST pEnum;
            HRESULT hr = pDrives->EnumObjects(0, SHCONTF_FOLDERS, &pEnum);
            if (SUCCEEDED(hr))
            {
                LPITEMIDLIST pidl;
                STRRET str;

                QString childPath;
                while(pEnum->Next(1, &pidl, 0) == S_OK)
                {
                    pDrives->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &str);
                    childPath = strToString(pidl, &str);

                    if(isRootPath(childPath) && filePath.startsWith(childPath))
                    {
                        item = ShellItem::Ptr(new ShellItem());
                        item->pidlRel = pidl;
                        pDrives->AddRef();
                        item->pParentFolder = pDrives;
                        break;
                    }
                }
                while(true)
                {
                    if(childPath == filePath)
                        break;

                   if(!getChildParent(item, childPath, filePath))
                       break;
                }
                pEnum->Release();
                if(item)
                {
                    if(isRoot)
                        return item->pParentFolder;

                    LPSHELLFOLDER pParentFolder = 0;
                    item->pParentFolder->BindToObject(item->pidlRel, 0, IID_IShellFolder,
                                                                                 (LPVOID*)&pParentFolder);
                    return pParentFolder;
                }
            }
        }
        return 0;
    }

    bool getChildParent(ShellItem::Ptr & item, QString &childPath, QString const& filePath)
    {
        if(!item)
            return  false;

        LPSHELLFOLDER  pParentFolder = NULL;
        HRESULT hr = item->pParentFolder->BindToObject(item->pidlRel, 0, IID_IShellFolder,
                                                             (LPVOID*)&pParentFolder);
        if(SUCCEEDED(hr))
        {
            LPENUMIDLIST   pEnum;
            hr = pParentFolder->EnumObjects(NULL, SHCONTF_FOLDERS, &pEnum);
            if(SUCCEEDED(hr))
            {
                LPITEMIDLIST   pidl;
                DWORD          dwFetched = 1;
                STRRET str;
                while(S_OK == (pEnum->Next(1, &pidl, &dwFetched)) && dwFetched)
                {
                     pParentFolder->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &str);
                     childPath = strToString(pidl, &str);

                     int index = filePath.indexOf(childPath);
                     if(index >= 0)
                     {
                        if(filePath == childPath || filePath.at(childPath.size()) == '\\')
                        {
                            ShellItem::Ptr newItem(new ShellItem());
                            newItem->pidlRel = pidl;
                            pParentFolder->AddRef();
                            newItem->pParentFolder = pParentFolder;
                            item = newItem;
                            pEnum->Release();
                            pParentFolder->Release();
                            return true;
                        }
                     }
                }
                pEnum->Release();
            }
            pParentFolder->Release();
        }
        return false;
    }

    ContextMenuItems sendToMenuItems()
    {
        ContextMenuItems items;
        LPSHELLFOLDER psf = getSpecialFolder(CSIDL_SENDTO);
        if(psf)
        {
            LPENUMIDLIST pEnum;
            HRESULT hr = psf->EnumObjects(0, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &pEnum);
            if (SUCCEEDED(hr))
            {
                LPITEMIDLIST pidl;
                STRRET str;
                while(pEnum->Next(1, &pidl, 0) == S_OK)
                {
                    ContextMenuItem item;

                    psf->GetDisplayNameOf(pidl, SHGDN_NORMAL, &str);
                    item.name = strToString(pidl, &str);
                    psf->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &str);
                    item.iconName = strToString(pidl, &str);
                    item.icon = getICon(item.iconName);
                    items << item;
                    CoTaskMemFree(pidl);
                }
                pEnum->Release();
            }
            psf->Release();
        }
        std::sort(items.begin(), items.end());
        return items;
    }

    void execSendToCmd(QStringList const& fileNames, QString const& name)
    {
        if(!open(fileNames))
            return;

        LPSHELLFOLDER psf = getSpecialFolder(CSIDL_SENDTO);
        if(!psf)
            return;
        LPENUMIDLIST peidl;
        HRESULT hr = psf->EnumObjects(0, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &peidl);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl = 0;
            STRRET str;
            while(peidl->Next(1, &pidl, 0) == S_OK)
            {
                psf->GetDisplayNameOf(pidl, SHGDN_NORMAL, &str);
                if(strToString(pidl, &str) == name)
                    break;
                CoTaskMemFree(pidl);
                pidl = 0;
            }
            if(pidl)
            {
                LPDROPTARGET pdt;
                hr = psf->GetUIObjectOf(0, 1, (PCUITEMID_CHILD_ARRAY)&pidl,
                                        IID_IDropTarget, 0, (LPVOID *)&pdt);
                if (SUCCEEDED(hr))
                {
                    doDrop(pDataObject, pdt);
                    pdt->Release();
                }
            }
        }
        close();
    }

    void doDrop(LPDATAOBJECT pdto, LPDROPTARGET pdt)
    {
        POINTL pt = { 0, 0 };
        DWORD dwEffect = DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;
        HRESULT hr = pdt->DragEnter(pdto, MK_LBUTTON, pt, &dwEffect);

        if (SUCCEEDED(hr) && dwEffect)
            pdt->Drop(pdto, MK_LBUTTON, pt, &dwEffect);
        else
            pdt->DragLeave();
    }

    QIcon getICon(QString const& fileName)
    {
        QFileInfo fileInfo(fileName.toLower());
        return QFileIconProvider().icon(fileInfo);
    }

    LPSHELLFOLDER getSpecialFolder(int idFolder)
    {
        LPITEMIDLIST pidl;
        LPSHELLFOLDER psf = NULL;

        HRESULT hr = SHGetSpecialFolderLocation(0, idFolder, &pidl);
        if (SUCCEEDED(hr))
        {
            pDesktop->BindToObject(pidl, NULL, IID_IShellFolder, (LPVOID *)&psf);
            CoTaskMemFree(pidl);
        }
        return psf;
    }

private:
    ContextMenuHelper()
    : pDesktop(0)
    , pidlDrives(0)
    , pDataObject(0)
    , size(0)
    {
        SHGetDesktopFolder(&pDesktop);
    }

    QString strToString(LPITEMIDLIST pidl, STRRET *str)
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
    IShellFolder* pDesktop;
    LPITEMIDLIST* pidlDrives;
    IDataObject* pDataObject;
    int size;
};

ContextMenu::ContextMenu()
{
}

ContextMenuItems ContextMenu::FileCommands()
{
    ContextMenuItems items;
    GetShellContextItems("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\*\\shell", items, true);
    GetShellContextItems("HKEY_CLASSES_ROOT\\*\\shell", items);
    GetShellContextItems("HKEY_CURRENT_USER\\SOFTWARE\\Classes\\*\\shell", items);

    //GetShellExContextItems("HKEY_CURRENT_USER\\SOFTWARE\\Classes\\*\\shellex\\ContextMenuHandlers", items);
    GetShellExContextItems("HKEY_CLASSES_ROOT\\*\\shellex\\ContextMenuHandlers", items);
    //GetShellExContextItems("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\*\\shellex\\ContextMenuHandlers", items);
    return items;
}

ContextMenuItems ContextMenu::DirCommands()
{
    ContextMenuItems items;
    GetShellContextItems("HKEY_CLASSES_ROOT\\Directory\\shell", items);
    return items;
}

ContextMenuItems ContextMenu::SendTo()
{
    return ContextMenuHelper::Instatnce()->sendToMenuItems();
}

void ContextMenu::show(QStringList const& fileNames, void *handle, QPoint const& p)
{
    ContextMenuHelper::Instatnce()->showContextMenu(fileNames, handle, p.x(), p.y());
}

void ContextMenu::GetShellContextItems(QString const& fileName,
                                       ContextMenuItems & items,
                                       bool isSort)
{
    QSettings reg(fileName, QSettings::NativeFormat);
    QStringList groups = reg.childGroups();

    if(isSort)
        std::sort(groups.begin(), groups.end(), std::greater<QString>());

    foreach(auto const& group, groups)
    {
        reg.beginGroup(group);

        QStringList keys = reg.childKeys();
        ContextMenuItem item;
        foreach(auto const& key, keys)
        {
            if(key == "." || key == "MUIVerb")
            {
                item.name = reg.value(key).toString();
                if(item.name.startsWith("@"))
                    item.name = GetName(item.name.mid(1));
            }
            else if(key == "Icon")
            {
                item.iconName = reg.value(key).toString();
                item.icon = GetIcon(item.iconName);
            }
        }

        QStringList childGroups = reg.childGroups();
        foreach(auto const& childGroup, childGroups)
        {
            reg.beginGroup(childGroup);
            if(childGroup == "command")
            {
                item.command = reg.value(".").toString();
                GetCommand(item.command, item);
            }
            reg.endGroup();
        }

        if(!item.command.isEmpty() && !item.name.isEmpty())
        {
            if(items.indexOf(item) < 0)
                items << item;
        }

        reg.endGroup();
    }
}

void ContextMenu::GetShellExContextItems(QString const& fileName,
                                         ContextMenuItems & items)
{
    QSettings reg(fileName, QSettings::NativeFormat);

    QStringList groups = reg.childGroups();
    foreach(auto const& group, groups)
    {
        reg.beginGroup(group);
        QStringList keys = reg.childKeys();
        ContextMenuItem item;
        foreach(auto const& key, keys)
        {
            if(key == ".")
            {
                QString value = reg.value(key).toString();
                GetShellExContextItem(
                            QString("HKEY_CLASSES_ROOT\\CLSID\\%1\\Settings").arg(value), item);
            }
        }
        if(!item.command.isEmpty() && !item.name.isEmpty())
        {
            if(items.indexOf(item) < 0)
                items << item;
        }
        reg.endGroup();
    }
}

void ContextMenu::GetShellExContextItem(QString const& fileName, ContextMenuItem & item)
{
    QSettings reg(fileName, QSettings::NativeFormat);
    QStringList keys = reg.childKeys();
    foreach(auto const& key, keys)
    {
        if(key == "Path")
        {
            item.command = reg.value(key).toString();
            item.icon = Utils::GetIcon(item.command, 0);
        }
        else if(key == "Title")
            item.name = reg.value(key).toString();
    }
    item.parameters = "%1";
}

void ContextMenu::GetShellExContextMenu(QString const& fileName, QString const& glsid,
                                        QString const& docFileName)
{
    QSettings reg(fileName, QSettings::NativeFormat);
    QStringList keys = reg.childKeys();
    foreach(auto const& key, keys)
    {
        if(key == ".")
        {
            QAxObject *mpAxObj =  new QAxObject();
            mpAxObj->setControl(glsid);
            QString doc = mpAxObj->generateDocumentation();
            QFile file(QString("%1.html").arg(docFileName));
            file.open(QIODevice ::ReadWrite|QIODevice ::Text);
            QTextStream ts(&file);
            ts << doc << endl;
            delete mpAxObj;
        }
    }
}

void ContextMenu::PrintCommand(ContextMenuItems const& items)
{
    for(auto const& item : items)
    {
        qDebug() << "----------------------------------";
        qDebug() << "Name: " << item.name;
        qDebug() << "IconName: " << item.iconName;
        qDebug() << "Command: " << item.command << item.parameters;
    }
}

void ContextMenu::GetCommand(QString const& command, ContextMenuItem &item)
{
    if(command.startsWith("\""))
    {
        QStringList items = command.split("\"");
        items.removeAt(0);
        item.command = items.first();
        items.removeAt(0);
        item.parameters = items.join("\"");
    }
    else
    {
        QStringList items = command.split(" ");
        item.command = items.first();
        items.removeAt(0);
        item.parameters = items.join(" ");
    }
}

QString ContextMenu::GetName(QString const& name)
{
    QStringList items = name.split(",");
    if(items.size() != 2)
        return QString();

    return Utils::GetText(items[0], -items[1].toInt());
}

static QString TrimName(QString const& name)
{
    QString newName = name;
    if(newName.startsWith("\""))
        newName = newName.mid(1);
    if(newName.endsWith("\""))
        newName = newName.left(newName.size() - 1);
    return newName;
}

QIcon ContextMenu::GetIcon(QString const& name)
{
    QStringList items = name.split(",");
    if(items.size() == 2)
        return Utils::GetIcon(TrimName(items[0]), items[1].toInt());
    else
        return Utils::GetIcon(TrimName(name), 0);
}

void ContextMenuItem::exec(QString const& fileName) const
{
    if(command.isEmpty())
    {
        ContextMenuHelper::Instatnce()->execSendToCmd(QStringList() << fileName, name);
        return;
    }

    QString params = parameters;
    QString newFileName = QString("\"%1\"").arg(fileName);

    params.replace("\"%1\"", newFileName)
            .replace("\"%L\"", newFileName)
            .replace("\"%V\"", newFileName)
            .replace("%L", newFileName)
            .replace("%V", newFileName)
            .replace("%1", newFileName);

    ShellExecute(0, L"open", command.toStdWString().c_str(),
                 params.toStdWString().c_str(), 0, SW_SHOWNORMAL);
}

void ContextMenuItem::exec(QStringList const& fileNames) const
{
    if(fileNames.isEmpty())
        return;

    if(command.isEmpty())
    {
        ContextMenuHelper::Instatnce()->execSendToCmd(fileNames, name);
        return;
    }

    QStringList newFileNames;
    foreach(auto const& fileName, fileNames)
    {
        newFileNames << QString("\"%1\"").arg(fileName);
    }

    QString newFileName = newFileNames.join(" ");
    QString params = parameters;

    params.replace("\"%1\"", newFileName)
            .replace("\"%L\"", newFileName)
            .replace("\"%V\"", newFileName)
            .replace("%L", newFileName)
            .replace("%V", newFileName)
            .replace("%1", newFileName);

    ShellExecute(0, L"open", command.toStdWString().c_str(),
                 params.toStdWString().c_str(), 0, SW_SHOWNORMAL);
}
