#ifndef REMOTEDIRMODEL_H
#define REMOTEDIRMODEL_H

#include "treemodel.h"
#include <QIcon>

class RemoteDirModel : public TreeModel
{
public:
    explicit RemoteDirModel(QObject *parent = nullptr);

protected:
    virtual TreeItem *createRootItem();
    virtual QVariant textAlignment(int column) const;
    virtual QVariant icon(const QModelIndex &index) const;
    virtual QVariant userData(const QModelIndex &index) const;
    virtual void setupModelData(TreeItem *parent);
    virtual QVariant foreColor(const QModelIndex &index) const;
private:
    QMap<QString, QIcon> iconMap;
    QIcon dirIcon;
    QIcon backIcon;
};

#endif // REMOTEDIRMODEL_H
