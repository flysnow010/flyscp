#ifndef COMPRESSDIRMODEL_H
#define COMPRESSDIRMODEL_H

#include "dirmodel.h"
#include "core/compressfileinfo.h"
#include <QIcon>

class QFileInfo;
class CompressDirModel : public DirModel
{
public:
    explicit CompressDirModel(QObject *parent = nullptr);

    void setCompressFile(QFileInfo const& fileInfo);
    bool setDir(QString const& dir);
    bool cd(QString const& dir);
    QString dir() const;
    CompressFileInfo::Ptr fileInfo(int index);

    bool rmFile(QString const& filePath);
    bool rename(QString const& oldFileName, QString const& newFileName);
    bool extract(QString const& targetPath, QString const& filePath, bool isWithPath);
    void refresh();
    bool setData(const QModelIndex &index, const QVariant &value, int role);
protected:
    virtual TreeItem *createRootItem();
    virtual QVariant textAlignment(const QModelIndex &index) const;
    virtual bool editable(const QModelIndex &index) const;
    virtual QVariant editText(const QModelIndex &index) const;
    virtual QVariant icon(const QModelIndex &index) const;
    virtual void setupModelData(TreeItem *parent);
private:
    CompressFile compressFile;
    CompressFileInfos fileInfos_;
    QIcon dirIcon;
    QIcon fileIcon;
    QIcon backIcon;
};

#endif // COMPRESSDIRMODEL_H
