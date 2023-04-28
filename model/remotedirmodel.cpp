#include "remotedirmodel.h"
#include "treeitem.h"
#include "util/utils.h"
#include <QBrush>
#include <QDateTime>

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
    fileInfos_ = dir_->fileInfoList(static_cast<ssh::Dir::Filter>(filter));
    setupData();
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
                    << QDateTime::fromSecsSinceEpoch(fileInfo->time()).toString("yyyy-MM-dd HH:mm:ss");

            if(!iconMap.contains(suffix))
                iconMap.insert(suffix, Utils::fileIcon(suffix));

        }
        else if(fileInfo->isDir())
        {
            rowData << QString() << QString("<DIR>")
                    << QDateTime::fromSecsSinceEpoch(fileInfo->time()).toString("yyyy-MM-dd HH:mm:ss");
        }
        TreeItem* item = new TreeItem(rowData, parent);
        parent->appendChild(item);
    }

}
