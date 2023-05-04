#include "remotedirmodel.h"
#include "treeitem.h"
#include "util/utils.h"
#include <QBrush>
#include <QDateTime>
namespace  {
int const NAME_INDEX = 0;
int const SUFFIX_INDEX = 1;
int const SIZE_INDEX = 2;
int const TIME_INDEX = 3;
}

RemoteDirModel::RemoteDirModel(QObject *parent)
    : TreeModel(parent)
    , dirIcon(Utils::dirIcon())
    , backIcon(":/image/back.png")
{
    setupData();
}

void RemoteDirModel::setDir(ssh::DirPtr const& dir)
{
    dir_ = dir;
    uint32_t filter = ssh::Dir::AllEntries | ssh::Dir::NoSymLinks | ssh::Dir::NoDot;
    if(dir_->isRoot())
        filter |= ssh::Dir::NoDotDot;
    fileInfos_ = dir_->fileInfoList(static_cast<ssh::Dir::Filter>(filter));
    setupData();
}

QString RemoteDirModel::dirName()
{
    if(dir_)
        return QString::fromStdString(std::string(dir_->dirName()));
    return QString();
}

void RemoteDirModel::sortItems(int index, bool isDescendingOrder)
{
   uint32_t sortFlag = ssh::Dir::DirsFirst;

    if(index == NAME_INDEX)
        sortFlag |= ssh::Dir::Name;
    else if(index == SIZE_INDEX)
        sortFlag |=  ssh::Dir::Size;
    else if(index == TIME_INDEX)
        sortFlag |=  ssh::Dir::Time;
    else if(index == SUFFIX_INDEX)
        sortFlag |= ssh::Dir::Type;
    if(!isDescendingOrder)
        sortFlag |=  ssh::Dir::Reversed;
    uint32_t filter = ssh::Dir::AllEntries | ssh::Dir::NoSymLinks | ssh::Dir::NoDot;
    if(dir_->isRoot())
        filter |= ssh::Dir::NoDotDot;
    fileInfos_ = dir_->fileInfoList(static_cast<ssh::Dir::Filter>(filter),
                                   static_cast<ssh::Dir::SortFlag>(sortFlag));
    setupData();
}

ssh::FileInfoPtr RemoteDirModel::fileInfo(int row)
{
    size_t index = static_cast<size_t>(row);
    if(index < fileInfos_.size())
        return fileInfos_.at(index);
    return ssh::FileInfoPtr();
}

std::string RemoteDirModel::filePath(const char* path)
{
    if(dir_)
    {
        std::string dir(dir_->dirName());
        if(dir_->isRoot())
            return dir + std::string(path);
        else
            return dir + std::string("/") + std::string(path);
    }
    return std::string(path);
}

std::string RemoteDirModel::parentPath()
{
    if(dir_)
    {
        std::string path(dir_->dirName());
        auto pos = path.find_last_of("/");
        if(pos == 0)
            pos += 1;
        return path.substr(0, pos);
    }
    return std::string();
}

TreeItem* RemoteDirModel::createRootItem()
{
    QList<QVariant> rootData;
    rootData << tr("Name") << tr("Suffix") << tr("Size") << tr("Date Modified") << tr("Property");
    TreeItem* item = new TreeItem(rootData);
    return item;
}

QVariant RemoteDirModel::icon(const QModelIndex &index) const
{
    if(index.column() != 0)
        return QVariant();
    auto const& fileInfo = fileInfos_[index.row()];
    if(fileInfo->isDir())
    {
        if(fileInfo->isParent())
            return backIcon;
        return dirIcon;
    }
    else if(fileInfo->isFile())
    {
        QString suffix = QString::fromStdString(fileInfo->suffix());
        return iconMap.value(suffix);
    }
    return QVariant();
}

QVariant RemoteDirModel::userData(const QModelIndex &index) const
{
    if(index.column() != 3)
        return QVariant();
    return QVariant();
}

QVariant RemoteDirModel::textAlignment(int column) const
{
    if(column == 0)
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    return QVariant();
}

QVariant RemoteDirModel::foreColor(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QBrush(QColor(QString("#454545")));
}

void RemoteDirModel::setupModelData(TreeItem *parent)
{
    for(auto const& fileInfo : fileInfos_)
    {
        QList<QVariant> rowData;
        rowData << QString::fromStdString(fileInfo->basename());
        if(fileInfo->isFile())
        {
            QString suffix = QString::fromStdString(fileInfo->suffix());
            rowData << suffix
                    << Utils::formatFileSizeB(fileInfo->size())
                    << QDateTime::fromSecsSinceEpoch(fileInfo->time()).toString("yyyy-MM-dd HH:mm:ss")
                    << property(fileInfo->permissions(), false);

            if(!iconMap.contains(suffix))
                iconMap.insert(suffix, Utils::fileIcon(suffix));

        }
        else if(fileInfo->isDir())
        {
            rowData << QString() << QString("<DIR>")
                    << QDateTime::fromSecsSinceEpoch(fileInfo->time()).toString("yyyy-MM-dd HH:mm:ss")
                    << property(fileInfo->permissions(), true);
        }
        TreeItem* item = new TreeItem(rowData, parent);
        parent->appendChild(item);
    }
}

QString RemoteDirModel::property(uint32_t permissions, bool isDir) const
{
    QString p;
    p += isDir   ? "d" : "-";
    p += (permissions & ssh::FileInfo::User_Read)   ? "r" : "-";
    p += (permissions & ssh::FileInfo::User_Write)  ? "w" : "-";
    p += (permissions & ssh::FileInfo::User_Exe)    ? "x" : "-";
    p += (permissions & ssh::FileInfo::Group_Read)  ? "r" : "-";
    p += (permissions & ssh::FileInfo::Group_Write) ? "w" : "-";
    p += (permissions & ssh::FileInfo::Group_Exe)   ? "x" : "-";
    p += (permissions & ssh::FileInfo::Other_Read)  ? "r" : "-";
    p += (permissions & ssh::FileInfo::Other_Write) ? "w" : "-";
    p += (permissions & ssh::FileInfo::Other_Exe)   ? "x" : "-";

    return p;
}
