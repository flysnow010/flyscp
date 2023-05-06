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
int const PROPERTY_INDEX = 4;
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
    refresh();
}

void RemoteDirModel::refresh()
{
    uint32_t filter = ssh::Dir::AllEntries | ssh::Dir::NoSymLinks | ssh::Dir::NoDot;
    ssh::Dir::SortFlag sortFlag = ssh::Dir::DirsFirst;
    if(dir_->isRoot())
        filter |= ssh::Dir::NoDotDot;
    fileInfos_ = dir_->fileInfoList(static_cast<ssh::Dir::Filter>(filter), sortFlag);
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
    else if(index == PROPERTY_INDEX)
        sortFlag |= ssh::Dir::Property;
    if(!isDescendingOrder)
        sortFlag |=  ssh::Dir::Reversed;
    uint32_t filter = ssh::Dir::AllEntries | ssh::Dir::NoSymLinks | ssh::Dir::NoDot;
    if(dir_->isRoot())
        filter |= ssh::Dir::NoDotDot;
    fileInfos_ = dir_->fileInfoList(static_cast<ssh::Dir::Filter>(filter),
                                   static_cast<ssh::Dir::SortFlag>(sortFlag));
    setupData();
}

ssh::FileInfoPtr RemoteDirModel::fileInfo(int row) const
{
    size_t index = static_cast<size_t>(row);
    try
    {
        return fileInfos_.at(index);
    }  catch (...) {

    }
    return ssh::FileInfoPtr();
}

std::string RemoteDirModel::filePath(const char* path) const
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

std::string RemoteDirModel::parentPath() const
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

bool RemoteDirModel::mkdir(std::string const& path)
{
    if(dir_)
    {
        std::string filepath = filePath(path.c_str());
        return dir_->mkdir(filepath.c_str());
    }
    return false;
}

bool RemoteDirModel::rmdir(std::string const& path)
{
    if(dir_)
    {
        std::string filepath = filePath(path.c_str());
        return dir_->rmdir(filepath.c_str());
    }
    return false;
}

bool RemoteDirModel::mkFile(std::string const& filename)
{
    if(dir_)
    {
        std::string filepath = filePath(filename.c_str());
        return dir_->mkFile(filepath.c_str());
    }
    return false;
}

bool RemoteDirModel::rmFile(std::string const& filename)
{
    if(dir_)
    {
        std::string filepath = filePath(filename.c_str());
        return dir_->rmFile(filepath.c_str());
    }
    return false;
}

bool RemoteDirModel::rename(std::string const& original, std::string const& newname)
{
    if(dir_)
    {
        std::string originalPath = filePath(original.c_str());
        std::string newnamePath = filePath(newname.c_str());
        return dir_->rename(originalPath.c_str(), newnamePath.c_str());
    }
    return false;
}

bool RemoteDirModel::chmod(const char *file, uint16_t mode)
{
    if(dir_)
    {
        std::string filename = filePath(file);
        return dir_->chmod(filename.c_str(), mode);
    }
    return false;
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

QVariant RemoteDirModel::textAlignment(const QModelIndex &index) const
{
    if(index.column() == NAME_INDEX)
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    else if(index.column() == SIZE_INDEX)
    {
        ssh::FileInfoPtr info = fileInfo(index.row());
        if(info && info->isFile())
            return int(Qt::AlignRight | Qt::AlignVCenter);
    }
    return QVariant();
}

QVariant RemoteDirModel::headerTextAlignment(int column) const
{
    if(column == NAME_INDEX)
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
                    << Utils::permissionsText(fileInfo->permissions(), false);

            if(!iconMap.contains(suffix))
                iconMap.insert(suffix, Utils::fileIcon(suffix));

        }
        else if(fileInfo->isDir())
        {
            rowData << QString() << QString("<DIR>")
                    << QDateTime::fromSecsSinceEpoch(fileInfo->time()).toString("yyyy-MM-dd HH:mm:ss")
                    << Utils::permissionsText(fileInfo->permissions(), true);
        }
        TreeItem* item = new TreeItem(rowData, parent);
        parent->appendChild(item);
    }
}

