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
    int fileCount () const { return fileCount_; }
    int dirCount() const { return dirCount_; }
    qint64 fileSizes() const { return fileSizes_; }
    void cancheIcon(QString const& suffix, QIcon const& icon);
    bool setData(const QModelIndex &index, const QVariant &value, int role);
protected:
    virtual TreeItem *createRootItem();
    virtual QVariant textAlignment(const QModelIndex &index) const;
    virtual bool editable(const QModelIndex &index) const;
    virtual QVariant editText(const QModelIndex &index) const;
    virtual QVariant headerTextAlignment(int column) const;
    virtual QVariant icon(const QModelIndex &index) const;
    virtual QVariant userData(const QModelIndex &index) const;
    virtual QVariant toolTip(const QModelIndex &index) const;
    virtual void setupModelData(TreeItem *parent);
private:
    ssh::DirPtr dir_;
    ssh::FileInfos fileInfos_;
    QMap<QString, QIcon> iconMap;
    QIcon dirIcon;
    QIcon backIcon;
    int fileCount_;
    int dirCount_;
    qint64 fileSizes_;
};

#endif // REMOTEDIRMODEL_H
