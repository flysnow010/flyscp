#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H
#include <QString>
#include <QIcon>
#include <QList>

struct ContextMenuItem
{
    QString name;
    QIcon icon;
    QString iconName;
    QString command;
    QString parameters;
    void exec(QString const& fileName) const;
    void exec(QStringList const& fileNames) const;
    bool operator == (ContextMenuItem const& r)
    {
        return this->name == r.name;
    }

    bool operator < (ContextMenuItem const& r)
    {
        return this->name < r.name;
    }
};

typedef QList<ContextMenuItem> ContextMenuItems;
class ContextMenu
{
public:
    ContextMenu();

    static ContextMenuItems FileCommands();
    static ContextMenuItems DirCommands();
    static ContextMenuItems SendTo();
    static void PrintCommand(ContextMenuItems const& items);
private:
    static void GetShellContextItems(QString const& fileName, ContextMenuItems & items, bool isSort = false);
    static void GetShellExContextItems(QString const& fileName, ContextMenuItems & items);
    static void GetShellExContextItem(QString const& fileName, ContextMenuItem & item);
    static void GetShellExContextMenu(QString const& fileName, QString const& glsid,
                                      QString const& docFileName);
    static void GetCommand(QString const& command, ContextMenuItem &item);
    static QString GetName(QString const& name);
    static QIcon   GetIcon(QString const& name);
};

#endif // CONTEXTMENU_H
