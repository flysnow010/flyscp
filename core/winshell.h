#ifndef WINSHELL_H
#define WINSHELL_H
#include <QIcon>
#include <QList>
#include "shellitem.h"

class QMimeData;
class QStringList;
class WinLibDir
{
public:
    QString caption;
    QString filePath;
    QString showPath() const;
    QIcon   icon() const;
};

class ShellMenuItem
{
public:
    QString caption;
    QString filePath;
    QIcon   icon;
    ShellItem::Ptr pItem;
    QString showPath() const { return QString("\\\\%1\\").arg(caption); }
    bool isDir() const { return filePath.startsWith("::") == false; }

    bool operator < (ShellMenuItem const& r)
    {
        return *pItem < *(r.pItem);
    }
    void exec() const { pItem->exec(); }
};

typedef QList<ShellMenuItem> ShellMenuItems;
class WinShell
{
public:
    WinShell();

    static void Property(QString const& fileName);
    static void Property(QStringList const& fileNames);
    static void Copy(QStringList const& fileNames, bool isMove);
    static QMimeData* dropMimeData(QStringList const& fileNames);
    static void Open(QString const& fileName);
    static void OpenWith(QString const& fileName);
    static void OpenByExplorer(QString const& fileName);
    static void Exec(QString const& appName, QStringList const& params);
    static void Exec(QString const& appName, QString const& params = QString());
    static bool CreateShortcut(QString const& linkFilePath,
                               QString const& targetFilePath);
    static QList<WinLibDir> winLibDirs();
    static ShellMenuItems shellMenuItems();
    static void shellSubMenuItems(ShellMenuItem const& item,
                                  ShellMenuItems & menuItems, bool isOnlyDir = true);
};

#endif // WINSHELL_H
