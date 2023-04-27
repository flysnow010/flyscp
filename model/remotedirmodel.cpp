#include "remotedirmodel.h"
#include "treeitem.h"
#include "util/utils.h"
#include <QBrush>

RemoteDirModel::RemoteDirModel(QObject *parent)
    : TreeModel(parent)
    , dirIcon(Utils::dirIcon())
    , backIcon(":/image/back.png")
{
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
    Q_UNUSED(parent)
}


