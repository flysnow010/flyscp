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

    ssh::FileInfoPtr fileInfo(QString const& fileName) const;
    ssh::FileInfoPtr fileInfo(int row) const;
    std::string fileName(int row) const;
    std::string filePath(int row) const;
    std::string filePath(const char* path) const;
    std::string parentPath() const;

    bool mkdir(std::string const& path);
    bool rmdir(std::string const& path);
    bool mkFile(std::string const& filename);
    bool rmFile(std::string const& filename);
    bool rename(std::string const& original, std::string const& newname);
    bool chmod(const char* file, uint16_t mode);
    void refresh();
protected:
    virtual TreeItem *createRootItem();
    virtual QVariant textAlignment(const QModelIndex &index) const;
    virtual QVariant headerTextAlignment(int column) const;
    virtual QVariant icon(const QModelIndex &index) const;
    virtual QVariant userData(const QModelIndex &index) const;
    virtual void setupModelData(TreeItem *parent);
    virtual QVariant foreColor(const QModelIndex &index) const;
private:
    ssh::DirPtr dir_;
    ssh::FileInfos fileInfos_;
    QMap<QString, QIcon> iconMap;
    QIcon dirIcon;
    QIcon backIcon;
};

#endif // REMOTEDIRMODEL_H
