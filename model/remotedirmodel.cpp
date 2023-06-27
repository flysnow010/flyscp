#include "remotedirmodel.h"
#include "treeitem.h"
#include "util/utils.h"
#include <QBrush>
#include <QDateTime>
#include <QFileIconProvider>

namespace  {
int const NAME_INDEX = 0;
int const SUFFIX_INDEX = 1;
int const SIZE_INDEX = 2;
int const TIME_INDEX = 3;
int const PROPERTY_INDEX = 4;
}

static void CancheIcon(RemoteDirModel const* model,
                       QString const& suffix,
                       QIcon const& icon)
{
    RemoteDirModel *m = (RemoteDirModel *)model;
    m->cancheIcon(suffix, icon);
}


RemoteDirModel::RemoteDirModel(QObject *parent)
    : DirModel(parent)
    , dirIcon(Utils::dirIcon())
    , backIcon(":/image/back.png")
    , fileCount_(0)
    , dirCount_(0)
    , fileSizes_(0)
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
    if(dir_->is_root())
        filter |= ssh::Dir::NoDotDot;
    fileInfos_ = dir_->fileinfos(static_cast<ssh::Dir::Filter>(filter), sortFlag);
    setupData();
}

QString RemoteDirModel::dirName()
{
    if(dir_)
        return QString::fromStdString(std::string(dir_->dirname()));
    return QString();
}

void RemoteDirModel::sortItems(int index, bool isDescendingOrder)
{
    uint32_t sortFlag = ssh::Dir::DirsFirst;

    if(index == NAME_INDEX)
    {
        if(dirSortIsByTime())
            sortFlag |=  ssh::Dir::Time;
        else
            sortFlag |= ssh::Dir::Name;
    }
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
    if(dir_->is_root())
        filter |= ssh::Dir::NoDotDot;
    fileInfos_ = dir_->fileinfos(static_cast<ssh::Dir::Filter>(filter),
                                   static_cast<ssh::Dir::SortFlag>(sortFlag));
    setupData();
}

ssh::FileInfoPtr RemoteDirModel::fileInfo(QString const& fileName) const
{
    std::string filename = fileName.toStdString();
    for(auto const& fileInfo: fileInfos_)
    {
        if(fileInfo->basename() == filename)
            return fileInfo;
    }
    return ssh::FileInfoPtr();
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

std::string RemoteDirModel::fileName(int row) const
{
    ssh::FileInfoPtr fileinfo = fileInfo(row);
    if(fileinfo)
    {
        return fileinfo->name();
    }
    return std::string();
}

std::string RemoteDirModel::filePath(int row) const
{
    ssh::FileInfoPtr fileinfo = fileInfo(row);
    if(fileinfo)
    {
        return filePath(fileinfo->name());
    }
    return std::string();
}

std::string RemoteDirModel::filePath(const char* path) const
{
    if(dir_)
    {
        std::string dir(dir_->dirname());
        if(dir_->is_root())
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
        std::string path(dir_->dirname());
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
        return dir_->mkfile(filepath.c_str());
    }
    return false;
}

bool RemoteDirModel::rmFile(std::string const& filename)
{
    if(dir_)
    {
        std::string filepath = filePath(filename.c_str());
        return dir_->rmfile(filepath.c_str());
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
    if(index.column() != 0 || iconShowType() == IconShowType::None)
        return QVariant();
    auto const& fileInfo = fileInfos_[index.row()];
    if(fileInfo->is_dir())
    {
        if(fileInfo->is_parent())
            return backIcon;
        return dirIcon;
    }
    else if(fileInfo->is_file())
    {
        if(iconShowType() == IconShowType::Standard)
            return QFileIconProvider().icon(QFileIconProvider::File);
        QString suffix = QString::fromStdString(fileInfo->suffix());
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

bool RemoteDirModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.column() == 0 && role == Qt::EditRole)
    {
        QString newName = value.toString();
        if(!newName.isEmpty())
         {
            ssh::FileInfoPtr fileInfo = fileInfos_.at(index.row());
            std::string oldFileName = fileInfo->name();
            std::string const& suffix = fileInfo->suffix();
            std::string newFileName;
            if(!isRenameBaseName() || suffix.empty())
                newFileName = newName.toStdString();
            else
               newFileName = QString("%1.%2").arg(newName, QString::fromStdString(suffix)).toStdString();
            if(rename(oldFileName, newFileName))
            {
                TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
                return item->setData(index.column(), value);
            }
        }
    }
    return false;
}

QVariant RemoteDirModel::userData(const QModelIndex &index) const
{
    if(index.column() != 3)
        return QVariant();
    return QVariant();
}

QVariant RemoteDirModel::toolTip(const QModelIndex &index) const
{
    if(index.column() == 0)
    {
        if(isShowToolTips())
            return fileInfos_[index.row()]->name();
        return QVariant();
    }
    return QVariant();
}

QVariant RemoteDirModel::textAlignment(const QModelIndex &index) const
{
    if(index.column() == NAME_INDEX)
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    else if(index.column() == SIZE_INDEX)
    {
        ssh::FileInfoPtr info = fileInfo(index.row());
        if(info && info->is_file())
            return int(Qt::AlignRight | Qt::AlignVCenter);
    }
    return QVariant();
}

bool RemoteDirModel::editable(const QModelIndex &index) const
{
    if(index.column() == 0 && !fileInfos_[index.row()]->is_parent())
        return true;
    return false;
}

QVariant RemoteDirModel::editText(const QModelIndex &index) const
{
    if(index.column() == 0)
    {
        if(!isRenameBaseName() || fileInfos_[index.row()]->basename().empty())
            return QString::fromStdString(fileInfos_[index.row()]->name());
        else
            return QString::fromStdString(fileInfos_[index.row()]->basename());
    }
    return QVariant();
}

QVariant RemoteDirModel::headerTextAlignment(int column) const
{
    if(column == NAME_INDEX)
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    return QVariant();
}

void RemoteDirModel::setupModelData(TreeItem *parent)
{
    fileCount_ = 0;
    dirCount_ = 0;
    fileSizes_ = 0;
    for(auto const& fileInfo : fileInfos_)
    {
        QList<QVariant> rowData;
        rowData << QString::fromStdString(fileInfo->basename());
        if(fileInfo->is_file())
        {
            QString suffix = QString::fromStdString(fileInfo->suffix());
            rowData << suffix
                    << Utils::formatFileSizeB(fileInfo->size())
                    << QDateTime::fromSecsSinceEpoch(fileInfo->time()).toString("yyyy-MM-dd HH:mm:ss")
                    << Utils::permissionsText(fileInfo->permissions(), false);
            fileCount_++;
            fileSizes_ += fileInfo->size();

        }
        else if(fileInfo->is_dir())
        {
            rowData << QString() << QString("<DIR>")
                    << QDateTime::fromSecsSinceEpoch(fileInfo->time()).toString("yyyy-MM-dd HH:mm:ss")
                    << Utils::permissionsText(fileInfo->permissions(), true);
            dirCount_++;
        }
        TreeItem* item = new TreeItem(rowData, parent);
        parent->appendChild(item);
    }
}

void RemoteDirModel::cancheIcon(QString const& suffix, QIcon const& icon)
{
    iconMap.insert(suffix, icon);
}
