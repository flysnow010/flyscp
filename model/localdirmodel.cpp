#include "localdirmodel.h"
#include "treeitem.h"
#include "util/utils.h"

#include <QColor>
#include <QBrush>
#include <QDateTime>
#include <QDir>
#include <QFileIconProvider>

#define ParentPath ".."
#define ExeSuffix  "exe"

namespace  {
int const NONE_INDEX = -1;
int const NAME_INDEX = 0;
int const SUFFIX_INDEX = 1;
int const SIZE_INDEX = 2;
int const TIME_INDEX = 3;
}


static void CancheIcon(LocalDirModel const* model,
                       QString const& suffix,
                       QIcon const& icon)
{
    LocalDirModel *m = (LocalDirModel *)model;
    m->cancheIcon(suffix, icon);
}

LocalDirModel::LocalDirModel(QObject *parent)
    : DirModel(parent)
    , dirIcon(Utils::dirIcon())
    , backIcon(":/image/back.png")
    , sortIndex(NONE_INDEX)
    , isDescending(false)
{
    setupData();
}

TreeItem* LocalDirModel::createRootItem()
{
    QList<QVariant> rootData;
    rootData << tr("Name") << tr("Suffix") << tr("Size") << tr("Date Modified");
    TreeItem* item = new TreeItem(rootData);
    return item;
}

QVariant LocalDirModel::icon(const QModelIndex &index) const
{
    if(index.column() != 0 || iconShowType() == IconShowType::None)
        return QVariant();

    if(fileInfos_[index.row()].isRoot())
    {
        if(iconShowType() == IconShowType::Standard)if(iconShowType() == IconShowType::Standard)
            QFileIconProvider().icon(QFileIconProvider::Drive);
        return QFileIconProvider().icon(fileInfos_[index.row()]);
    }
    else if(fileInfos_[index.row()].isSymLink())
    {
        if(iconShowType() == IconShowType::ALLWithExeAndLink)
            return QFileIconProvider().icon(fileInfos_[index.row()]);
        QString suffix = fileInfos_[index.row()].suffix().toLower();
        if(!iconMap.contains(suffix))
        {
            QIcon icon = Utils::fileIcon(suffix);
            CancheIcon(this, suffix, icon);
            return icon;
        }
        return iconMap.value(suffix);
    }
    else if(fileInfos_[index.row()].fileName() == ParentPath)
        return backIcon;
    else if(fileInfos_[index.row()].isDir())
        return dirIcon;
    else if(fileInfos_[index.row()].isFile())
    {
        if(iconShowType() == IconShowType::Standard)
            return QFileIconProvider().icon(QFileIconProvider::File);

        QString suffix = fileInfos_[index.row()].suffix().toLower();
        if(suffix == ExeSuffix && iconShowType() == IconShowType::ALLWithExeAndLink)
        {
            QIcon icon = Utils::GetIcon(fileInfos_[index.row()].filePath());
            if(!icon.isNull())
                return icon;
        }
        if(!iconMap.contains(suffix))
        {
            QIcon icon = Utils::fileIcon(suffix);
            CancheIcon(this, suffix, icon);
            return icon;
        }
        return iconMap.value(suffix);
    }
    return QVariant();
}

bool LocalDirModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.column() == 0 && role == Qt::EditRole)
    {
        QString newName = value.toString();
        if(!newName.isEmpty())
         {
            QFileInfo fileInfo = fileInfos_.at(index.row());
            QString oldFileName = fileInfo.fileName();
            QString suffix = fileInfo.suffix();
            QString newFileName;
            if(!isRenameBaseName() || suffix.isEmpty())
                newFileName = newName;
            else
               newFileName = QString("%1.%2").arg(newName, suffix);
            if(rename(oldFileName, newFileName))
            {
                TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
                return item->setData(index.column(), value);
            }
        }
    }
    return false;
}

QVariant LocalDirModel::userData(const QModelIndex &index) const
{
    if(index.column() != 3)
        return QVariant();
    return fileInfos_[index.row()].size();
}

QVariant LocalDirModel::toolTip(const QModelIndex &index) const
{
    if(index.column() == 0)
    {
        if(isShowToolTips())
            return fileInfos_[index.row()].fileName();
        return QVariant();
    }
    return QVariant();
}

QVariant LocalDirModel::textAlignment(const QModelIndex &index) const
{
    if(index.column() == NAME_INDEX)
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    else if(index.column() == SIZE_INDEX)
    {
        if(fileInfos_.at(index.row()).isFile())
            return int(Qt::AlignRight | Qt::AlignVCenter);
    }
    return QVariant();
}

QVariant LocalDirModel::headerTextAlignment(int column) const
{
    if(column == NAME_INDEX)
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    return QVariant();
}

bool LocalDirModel::editable(const QModelIndex &index) const
{
    if(index.column() == 0 && fileInfos_[index.row()].fileName() != ParentPath)
        return true;
    return false;
}

QVariant LocalDirModel::editText(const QModelIndex &index) const
{
    if(index.column() == 0)
    {
        if(!isRenameBaseName() || fileInfos_[index.row()].completeBaseName().isEmpty())
            return fileInfos_[index.row()].fileName();
        else
            return fileInfos_[index.row()].completeBaseName();
    }
    return QVariant();
}

void LocalDirModel::setDir(QString const& dir)
{
    if(dir.isEmpty())
    {
        fileInfos_ = QDir::drives();
        modifyFileInfos(fileInfos_);
        setupData();
    }
    else
    {
        dir_.setPath(dir);
        refresh();
    }
}

void LocalDirModel::refresh()
{
    if(sortIndex != NONE_INDEX)
        sortItems(sortIndex, isDescending);
    else
        defaultRefresh();
}

void LocalDirModel::sortItems(int index, bool isDescendingOrder)
{
    QDir::SortFlags sortFlag = QDir::DirsFirst;

    if(index == NAME_INDEX)
    {
        if(dirSortIsByTime())
            sortFlag |=  QDir::SortFlag::Time;
        else
            sortFlag |= QDir::SortFlag::Name;
    }
    else if(index == SIZE_INDEX)
        sortFlag |=  QDir::SortFlag::Size;
    else if(index == TIME_INDEX)
        sortFlag |=  QDir::SortFlag::Time;
    else if(index == SUFFIX_INDEX)
        sortFlag |= QDir::SortFlag::Type;
    if(!isDescendingOrder)
        sortFlag |=  QDir::SortFlag::Reversed;
    fileInfos_ = dir_.entryInfoList(getFilters(), sortFlag);
    sortIndex = index;
    isDescending = isDescendingOrder;
    modifyFileInfos(fileInfos_);
    setupData();
}

QString LocalDirModel::dir() const
{
    return dir_.path();
}

bool LocalDirModel::mkdirs(QString const& dir)
{
    return dir_.mkpath(dir);
}

bool LocalDirModel::rename(QString const& original, QString const& newname)
{
    return dir_.rename(original, newname);
}

QString LocalDirModel::fileName(int index)
{
    return fileInfos_.at(index).fileName();
}

QFileInfo const& LocalDirModel::fileInfo(int index)
{
    return fileInfos_.at(index);
}

QString LocalDirModel::filePath(int index)
{
    return fileInfos_.at(index).filePath();
}

QString LocalDirModel::filePath(QString const& fileName)
{
    return dir_.filePath(fileName);
}

bool LocalDirModel::isParent(int index) const
{
    return fileInfos_[index].fileName() == ParentPath;
}

bool LocalDirModel::isDir(int index) const
{
    return fileInfos_[index].isDir();
}

bool LocalDirModel::isFile(int index) const
{
    return fileInfos_[index].isFile();
}

qint64 LocalDirModel::fileSize(int index) const
{
    return fileInfos_[index].size();
}

int LocalDirModel::indexOfFile(QString const& fileName)
{
    for(int i = 0; i < fileInfos_.size(); i++)
    {
        if(fileInfos_[i].fileName() == fileName)
            return i;
    }
    return -1;
}

bool LocalDirModel::cd(const QString &dirName)
{
    if(!dir_.cd(dirName))
    {
        if(dirName != ParentPath)
            return false;
        dir_= QDir();
        fileInfos_ = QDir::drives();
        setupData();
        return true;
    }

    fileInfos_ = dir_.entryInfoList(getFilters(),
                                    QDir::DirsFirst);
    modifyFileInfos(fileInfos_);
    setupData();
    return true;
}

void LocalDirModel::setupModelData(TreeItem *parent)
{
    fileCount_ = 0;
    dirCount_ = 0;
    fileSizes_ = 0;
    for(int i = 0; i < fileInfos_.size();i++)
    {
        QList<QVariant> rowData;
        if(fileInfos_[i].isRoot())
            rowData << fileInfos_[i].filePath();
        else if(fileInfos_[i].isSymLink())
                rowData << fileInfos_[i].completeBaseName();
        else if(fileInfos_[i].isDir())
            rowData << fileInfos_[i].fileName();
        else
        {
            QString baseName = fileInfos_[i].completeBaseName();
            if(baseName.isEmpty())
                rowData << fileInfos_[i].fileName();
            else
                rowData << baseName;
        }
        if(fileInfos_[i].isFile() || fileInfos_[i].isSymLink())
        {
            QString suffix;
            if(!fileInfos_[i].completeBaseName().isEmpty())
                suffix = fileInfos_[i].suffix();

            qint64 size = fileInfos_[i].size();
            rowData << suffix;
            if(size == 0 && fileInfos_[i].isSymLink())
                rowData << QString("<DIR>");
            else
                rowData    << Utils::formatFileSizeB(fileInfos_[i].size());
            rowData << fileInfos_[i].lastModified().toString("yyyy-MM-dd HH:mm:ss");

            fileCount_++;
            fileSizes_ += size;
        }
        else if(fileInfos_[i].isDir())
        {
            rowData << QString() << QString("<DIR>")
                    << fileInfos_[i].lastModified().toString("yyyy-MM-dd HH:mm:ss");
            if(!isParent(i))
                dirCount_++;
        }
        TreeItem* item = new TreeItem(rowData, parent);
        parent->appendChild(item);
    }
}

void LocalDirModel::modifyFileInfos(QFileInfoList &fileInfos)
{
    if(!isShowHidden())
    {
        for(int i = 0; i < fileInfos.size(); )
        {
            QString fileName = fileInfos[i].fileName();
            if(!fileName.startsWith(".")
                    || fileName == ParentPath)
                i++;
            else
                fileInfos.removeAt(i);
        }
    }
    int dirIndex = -1;
    for(int i = 0; i < fileInfos.size(); i++)
    {
        if(fileInfos[i].fileName() == ParentPath && i != 0)
        {
            QFileInfo filInfo = fileInfos[i];
            fileInfos.removeAt(i);
            fileInfos.push_front(filInfo);
        }
        if(fileInfos[i].isDir())
            dirIndex = i;
    }

    for(int i = 0; i <= dirIndex; i++)
    {
        if(fileInfos[i].isDir() && fileInfos[i].isSymLink())
        {
            if(i != dirIndex)
            {
                QFileInfo fileInfo = fileInfos[i];
                fileInfos.removeAt(i);
                fileInfos.insert(dirIndex, fileInfo);
            }
        }
    }
    if(isShowParentInRoot() && !fileInfos.isEmpty())
    {
        if(fileInfos[0].fileName() != ParentPath)
        {
            QFileInfo fileInfo(ParentPath);
            fileInfos.insert(0, fileInfo);
        }
    }
}

void LocalDirModel::defaultRefresh()
{
    fileInfos_ = dir_.entryInfoList(getFilters(),
                                    QDir::DirsFirst);
    modifyFileInfos(fileInfos_);
    setupData();
}

QDir::Filters LocalDirModel::getFilters()
{
    QDir::Filters filters = QDir::AllEntries | QDir::NoDot;
    if(isShowHidden())
        filters |= QDir::Hidden;
    if(isShowSystem())
        filters |= QDir::System;
    return filters;
}

void LocalDirModel::cancheIcon(QString const& suffix, QIcon const& icon)
{
    iconMap.insert(suffix, icon);
}
