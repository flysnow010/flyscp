#ifndef TREE_ITEM_H
#define TREE_ITEM_H

#include <QList>
#include <QVariant>

class TreeItem
{
public:
    TreeItem(const QList<QVariant> &data,
             TreeItem *parent = 0);
    ~TreeItem();

    void appendChild(TreeItem *child);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool setData(int column,
                 QVariant const& value);
    int row() const;
    TreeItem *parent();

private:
    QList<TreeItem*> childItems;
    QList<QVariant> itemData;
    TreeItem *parentItem;
};
#endif
