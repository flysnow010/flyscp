#ifndef TREE_MODEL_H
#define TREE_MODEL_H
#include "treeitem.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class TreeItem;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TreeModel(QObject *parent = 0);
    virtual ~TreeModel();

    void reset();
    QVariant data(const QModelIndex &index,
                  int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    TreeItem* index(QModelIndex const& index) const;
    QModelIndex index(int row,
                      int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

protected:
    void setupData();
    virtual TreeItem *createRootItem() = 0;
    virtual QVariant foreColor(const QModelIndex &index) const;
    virtual bool editable(const QModelIndex &index) const;
    virtual QVariant editText(const QModelIndex &index) const;
    virtual QVariant backColor(const QModelIndex &index) const;
    virtual QVariant textAlignment(const QModelIndex &index) const;
    virtual QVariant headerTextAlignment(int col) const;
    virtual QVariant icon(const QModelIndex &index) const;
    virtual QVariant userData(const QModelIndex &index) const;
    virtual QVariant toolTip(const QModelIndex &index) const;
    virtual void setupModelData(TreeItem *parent) = 0;
protected:
    TreeItem *rootItem;
};

#endif
