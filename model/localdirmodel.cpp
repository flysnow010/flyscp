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


LocalDirModel::LocalDirModel(QObject *parent)
    : TreeModel(parent)
    , dirIcon(Utils::dirIcon())
    , backIcon(":/image/back.png")
    , sortIndex(NONE_INDEX)
    , fileCount_(0)
    , dirCount_(0)
    , fileSizes_(0)
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
    if(index.column() != 0)
        return QVariant();
    if(fileInfos_[index.row()].isRoot() || fileInfos_[index.row()].isSymLink())
        return QFileIconProvider().icon(fileInfos_[index.row()]);
    else if(fileInfos_[index.row()].fileName() == ParentPath)
        return backIcon;
    else if(fileInfos_[index.row()].isDir())
        return dirIcon;
    else if(fileInfos_[index.row()].isFile())
    {
        QString suffix = fileInfos_[index.row()].suffix().toLower();
        if(suffix == ExeSuffix)
        {
            QIcon icon = Utils::GetIcon(fileInfos_[index.row()].filePath());
            if(!icon.isNull())
                return icon;
        }
        return iconMap.value(suffix);
    }
    return QVariant();
}

QVariant LocalDirModel::userData(const QModelIndex &index) const
{
    if(index.column() != 3)
        return QVariant();
    return fileInfos_[index.row()].size();
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
    Q_UNUSED(index)
    return false;
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
        sortFlag |= QDir::SortFlag::Name;
    else if(index == SIZE_INDEX)
        sortFlag |=  QDir::SortFlag::Size;
    else if(index == TIME_INDEX)
        sortFlag |=  QDir::SortFlag::Time;
    else if(index == SUFFIX_INDEX)
        sortFlag |= QDir::SortFlag::Type;
    if(!isDescendingOrder)
        sortFlag |=  QDir::SortFlag::Reversed;
    fileInfos_ = dir_.entryInfoList(QDir::AllEntries | QDir::NoDot, sortFlag);
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
        return false;

    fileInfos_ = dir_.entryInfoList(QDir::AllEntries | QDir::NoDot,
                                    QDir::DirsFirst);
    modifyFileInfos(fileInfos_);
    setupData();
    return true;
}

QVariant LocalDirModel::foreColor(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QBrush(QColor(QString("#454545")));
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

            if(!iconMap.contains(suffix))
                iconMap.insert(suffix, Utils::fileIcon(suffix));
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
                QFileInfo filInfo = fileInfos[i];
                fileInfos.removeAt(i);
                fileInfos.insert(dirIndex, filInfo);
            }
        }
    }
}

void LocalDirModel::defaultRefresh()
{
    fileInfos_ = dir_.entryInfoList(QDir::AllEntries | QDir::NoDot,
                                    QDir::DirsFirst);
    modifyFileInfos(fileInfos_);
    setupData();
}
