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
    bool cd(QString const& dir);

    CompressFileInfo::Ptr fileInfo(int index);
protected:
    virtual TreeItem *createRootItem();
    virtual QVariant textAlignment(const QModelIndex &index) const;
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
