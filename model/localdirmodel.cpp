#include "localdirmodel.h"
#include "treeitem.h"
#include "util/utils.h"

#include <QColor>
#include <QBrush>
#include <QDateTime>
#include <QDir>
#include <QFileIconProvider>
namespace  {
int const NAME_INDEX = 0;
int const SUFFIX_INDEX = 1;
int const SIZE_INDEX = 2;
int const TIME_INDEX = 3;
}


LocalDirModel::LocalDirModel(QObject *parent)
    : TreeModel(parent)
    , dirIcon(Utils::dirIcon())
    , backIcon(":/image/back.png")
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
    {
        QFileIconProvider fip;
        return fip.icon(fileInfos_[index.row()]);
    }
    else if(fileInfos_[index.row()].fileName() == "..")
        return backIcon;
    else if(fileInfos_[index.row()].isDir())
        return dirIcon;
    else if(fileInfos_[index.row()].isFile())
    {
        QString suffix = fileInfos_[index.row()].suffix();
        if(suffix == "exe")
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

QVariant LocalDirModel::textAlignment(int column) const
{
    if(column == 0)
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    return QVariant();
}

void LocalDirModel::setDir(QString const& dir)
{
    if(dir.isEmpty())
        fileInfos_ = QDir::drives();
    else
    {
        dir_.setPath(dir);
        fileInfos_ = dir_.entryInfoList(QDir::AllEntries | QDir::NoDot,
                                        QDir::DirsFirst);
    }
    setupData();
}

void LocalDirModel::update()
{
    fileInfos_ = dir_.entryInfoList(QDir::AllEntries | QDir::NoDot,
                                    QDir::DirsFirst);
    setupData();
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
    modifyFileInfos(fileInfos_);
    setupData();
}

QString LocalDirModel::dir()
{
    return dir_.path();
}

QString LocalDirModel::fileName(int index)
{
    return fileInfos_.at(index).fileName();
}

QString LocalDirModel::filePath(int index)
{
    return fileInfos_.at(index).filePath();
}

QString LocalDirModel::filePath(QString const& fileName)
{
    return dir_.filePath(fileName);
}

bool LocalDirModel::cd(const QString &dirName)
{
    if(!dir_.cd(dirName))
        return false;

    fileInfos_ = dir_.entryInfoList(QDir::AllEntries | QDir::NoDot,
                                    QDir::DirsFirst);
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
    iconMap.clear();

    for(int i = 0; i < fileInfos_.size();i++)
    {
        QList<QVariant> rowData;
        if(fileInfos_[i].isRoot())
            rowData << fileInfos_[i].filePath();
        else if(fileInfos_[i].isDir())
            rowData << fileInfos_[i].fileName();
        else
            rowData << fileInfos_[i].baseName();
        if(fileInfos_[i].isFile())
        {
            QString suffix = fileInfos_[i].suffix();
            rowData << suffix
                    << Utils::formatFileSizeB(fileInfos_[i].size())
                    << fileInfos_[i].lastModified().toString("yyyy-MM-dd HH:mm:ss");

            if(!iconMap.contains(suffix))
                iconMap.insert(suffix, Utils::fileIcon(suffix));

        }
        else if(fileInfos_[i].isDir())
        {
            rowData << QString() << QString("<DIR>")
                    << fileInfos_[i].lastModified().toString("yyyy-MM-dd HH:mm:ss");
        }
        TreeItem* item = new TreeItem(rowData, parent);
        parent->appendChild(item);
    }
}

void LocalDirModel::modifyFileInfos(QFileInfoList &fileInfos)
{
    for(int i = 0; i < fileInfos.size(); i++)
    {
        if(fileInfos[i].fileName() == ".." && i != 0)
        {
            QFileInfo filInfo = fileInfos[i];
            fileInfos.removeAt(i);
            fileInfos.push_front(filInfo);
            break;
        }
    }
}