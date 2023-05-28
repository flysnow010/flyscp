#include "fileuncompresser.h"
#include "winshell.h"
#include "util/utils.h"

#include <QStringList>

QString UncompressParam::overwriteMode() const
{
    if(isWithPath)
        return QString();

    if(mode == OverWrite)
        return QString(" -aoa");
    else if(mode == Skip)
        return QString(" -aos");
    else if(mode == AutoRename)
        return QString(" -aot");
    return QString();
}

FileUncompresser::FileUncompresser(QObject *parent)
    : QObject(parent)
{
}

bool FileUncompresser::uncompress(QStringList const& fileNames,
                UncompressParam const& param,
                QString const& targetFilePath)
{
    QString app = Utils::compressApp();
    foreach(auto const& fileName, fileNames)
    {
        QString newTargetPath;

        if(!param.isCreateDir)
            newTargetPath = targetFilePath;
        else
        {
            QDir dir(targetFilePath);
            QString pathName = QFileInfo(fileName).baseName();
            dir.mkdir(pathName);
            newTargetPath = dir.filePath(pathName);
        }

        QString params = QString("%1 %2 -o%3 %4 -r")
                .arg(param.isWithPath ? "x" : "e",
                     fileName, newTargetPath, param.filter);
        if(!param.isWithPath)
            params = params + param.overwriteMode();
        WinShell::Exec(app, params);
    }
    return true;
}
