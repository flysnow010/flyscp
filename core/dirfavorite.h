#ifndef DIRFAVORITE_H
#define DIRFAVORITE_H
#include <QList>

struct FavoriteItem
{
    QString caption;
    QString command;
    QString filePath;

    bool operator==(FavoriteItem const& item) {
        return item.filePath == filePath;
    }
    bool isCommand() const { return !command.isEmpty(); }
    QString getCommand() const
    {
        return QString("%1/%2").arg(filePath, command);
    }
};

class DirFavorite
{
public:
    DirFavorite();
    enum { MAX_SIZE = 25 };

    int size() const { return favoriteItems_.size(); }

    FavoriteItem* item(int index)
    {
        if(index >= 0 && index < favoriteItems_.size())
            return &favoriteItems_[index];
        return 0;
    }

    inline void addItem(FavoriteItem const& item)
    {
        if(favoriteItems_.size() >= MAX_SIZE)
            favoriteItems_.takeFirst();
        favoriteItems_ << item;
    }

    inline void upMove(int index)
    {
        int newIndex = index - 1;
        if(newIndex >= 0 && index < favoriteItems_.size())
        {
            FavoriteItem item = favoriteItems_.at(newIndex);
            favoriteItems_[newIndex] = favoriteItems_.at(index);
            favoriteItems_[index] = item;
        }
    }
    inline void downMove(int index)
    {
        int newIndex = index + 1;
        if(newIndex > 0 && newIndex < favoriteItems_.size())
        {
            FavoriteItem item = favoriteItems_.at(newIndex);
            favoriteItems_[newIndex] = favoriteItems_.at(index);
            favoriteItems_[index] = item;
        }
    }
    inline void removeItem(int index)
    {
        favoriteItems_.removeAt(index);
    }
    inline void removeItem(FavoriteItem const& item)
    {
        int index = favoriteItems_.indexOf(item);
        favoriteItems_.removeAt(index);
    }

    inline void setFavoriteItems(QList<FavoriteItem> const& items) { favoriteItems_ = items; }
    inline QList<FavoriteItem> const& favoriteItems() const { return favoriteItems_; }
private:
    QList<FavoriteItem> favoriteItems_;
};

#endif // DIRFAVORITE_H
