#ifndef FAVORITEDIRMODEL_H
#define FAVORITEDIRMODEL_H
#include "treemodel.h"

class DirFavorite;
class FavoriteItem;
class FavoriteDirModel : public TreeModel
{
    Q_OBJECT
public:
    explicit FavoriteDirModel(QObject *parent = nullptr);

    void setDirFavorite(DirFavorite* manager);
    void refresh();

    FavoriteItem* item(int row);

    bool setData(const QModelIndex &index,
                 const QVariant &value, int role) override;
protected:
    TreeItem *createRootItem() override;
    virtual void setupModelData(TreeItem *parent) override;
    virtual bool editable(const QModelIndex &index) const override;
private:
    DirFavorite* dirFavorite;
};

#endif // FAVORITEDIRMODEL_H
