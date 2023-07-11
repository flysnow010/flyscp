#include "favoritedirmodel.h"
#include "core/dirfavorite.h"

FavoriteDirModel::FavoriteDirModel(QObject *parent)
    : TreeModel(parent)
    , dirFavorite(0)
{
    setupData();
}

void FavoriteDirModel::setDirFavorite(DirFavorite* manager)
{
    dirFavorite = manager;
    setupData();
}

TreeItem *FavoriteDirModel::createRootItem()
{
    QList<QVariant> rootData;
    rootData << tr("Name");
    TreeItem* item = new TreeItem(rootData);
    return item;
}

bool FavoriteDirModel::setData(const QModelIndex &index,
                               const QVariant &value, int role)
{
    if(index.column() == 0 && role == Qt::EditRole)
    {
        QString newName = value.toString();
        if(!newName.isEmpty())
         {
            FavoriteItem* fi = dirFavorite->item(index.row());
            fi->caption = newName;
            TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
            return item->setData(index.column(), value);
        }
    }
    return false;
}

FavoriteItem* FavoriteDirModel::item(int row)
{
    return dirFavorite->item(row);
}

void FavoriteDirModel::refresh()
{
    setupData();
}

bool FavoriteDirModel::editable(const QModelIndex &index) const
{
    if(index.column() == 0)
        return true;
    return false;
}

void FavoriteDirModel::setupModelData(TreeItem *parent)
{
    if(!dirFavorite)
        return;

    for(int i = 0; i < dirFavorite->size(); i++)
    {
        QList<QVariant> rowData;
        FavoriteItem* fi = dirFavorite->item(i);
        rowData << fi->caption;
        TreeItem* item = new TreeItem(rowData, parent);
        parent->appendChild(item);
    }
}
