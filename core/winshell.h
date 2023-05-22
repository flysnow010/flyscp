#ifndef WINSHELL_H
#define WINSHELL_H
#include <QIcon>
#include <QList>

class QString;
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
    static QList<WinLibDir> winLibDirs();
};

#endif // WINSHELL_H
