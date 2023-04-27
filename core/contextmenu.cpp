#include "contextmenu.h"
#include "util/utils.h"
#include <windows.h>
#include <QSettings>
#include <QDebug>
#include <QAxObject>
#include <algorithm>

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

void ContextMenu::GetShellExContextItems(QString const& fileName,ContextMenuItems & items)
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
                //qDebug() << group;
                GetShellExContextItem(
                            QString("HKEY_CLASSES_ROOT\\CLSID\\%1\\Settings").arg(value), item);
//                GetShellExContextMenu(QString("HKEY_CLASSES_ROOT\\CLSID\\%1\\InprocServer32").arg(value),
//                                      value, group);
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
            QString dllName = reg.value(key).toString();
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
    QString params = parameters;
    params.replace("%1", fileName)
            .replace("%L", fileName)
            .replace("%V", fileName)
            .replace("%1", fileName);
    ShellExecute(0, L"open", command.toStdWString().c_str(),
                 params.toStdWString().c_str(), 0, SW_SHOWNORMAL);
}
