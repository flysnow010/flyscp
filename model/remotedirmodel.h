#ifndef REMOTEDIRMODEL_H
#define REMOTEDIRMODEL_H

#include "treemodel.h"
#include "ssh/dir.h"
#include "ssh/fileinfo.h"
#include <QIcon>

class RemoteDirModel : public TreeModel
{
public:
    explicit RemoteDirModel(QObject *parent = nullptr);

    void setDir(ssh::DirPtr const& dir);
    QString dirName();

    void sortItems(int index, bool isDescendingOrder);

    ssh::FileInfoPtr fileInfo(int row);
    std::string filePath(const char* path);
    std::string parentPath();
protected:
    virtual TreeItem *createRootItem();
    virtual QVariant textAlignment(int column) const;
    virtual QVariant icon(const QModelIndex &index) const;
    virtual QVariant userData(const QModelIndex &index) const;
    virtual void setupModelData(TreeItem *parent);
    virtual QVariant foreColor(const QModelIndex &index) const;
private:
    QString property(uint32_t permissions, bool isDir) const;
private:
    ssh::DirPtr dir_;
    ssh::FileInfos fileInfos_;
    QMap<QString, QIcon> iconMap;
    QIcon dirIcon;
    QIcon backIcon;
};

#endif // REMOTEDIRMODEL_H
