#ifndef DIRFAVORITE_H
#define DIRFAVORITE_H
#include <QList>
struct FavoriteItem
{
    QString caption;
    QString command;
    QString fileName;
    bool operator==(FavoriteItem const& item)
    {
        return item.fileName == fileName;
    }
};

class DirFavorite
{
public:
    DirFavorite();
    enum { MAX_SIZE = 25 };

    void addItem(FavoriteItem const& item)
    {
        if(favoriteItems_.size() >= MAX_SIZE)
            favoriteItems_.takeFirst();
        favoriteItems_ << item;

    }

    void removeItem(FavoriteItem const& item)
    {
        int index = favoriteItems_.indexOf(item);
        favoriteItems_.takeAt(index);
    }

    void setFavoriteItems(QList<FavoriteItem> const& items) { favoriteItems_ = items; }
    QList<FavoriteItem> const& favoriteItems() const { return favoriteItems_; }
private:
    QList<FavoriteItem> favoriteItems_;
};

#endif // DIRFAVORITE_H
