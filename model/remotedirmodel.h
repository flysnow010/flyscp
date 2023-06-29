#ifndef REMOTEDIRMODEL_H
#define REMOTEDIRMODEL_H

#include "dirmodel.h"
#include "ssh/dir.h"
#include "ssh/fileinfo.h"
#include <QIcon>

class RemoteDirModel : public DirModel
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
    void cancheIcon(QString const& suffix, QIcon const& icon);

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool isParent(int index) const override;
    bool isDir(int index) const override;
    bool isFile(int index) const override;
    qint64 fileSize(int index) const override;
protected:
    TreeItem *createRootItem() override;
    QVariant textAlignment(const QModelIndex &index) const override;
    bool editable(const QModelIndex &index) const override;
    QVariant editText(const QModelIndex &index) const override;
    QVariant headerTextAlignment(int column) const override;
    QVariant icon(const QModelIndex &index) const override;
    QVariant userData(const QModelIndex &index) const override;
    QVariant toolTip(const QModelIndex &index) const override;
    void setupModelData(TreeItem *parent) override;
private:
    uint32_t getFilters();
private:
    ssh::DirPtr dir_;
    ssh::FileInfos fileInfos_;
    QMap<QString, QIcon> iconMap;
    QIcon dirIcon;
    QIcon backIcon;
};

#endif // REMOTEDIRMODEL_H
