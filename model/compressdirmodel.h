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

    void setCompressFileName(QString const& fileName);
    QString compressFileName() const;
    bool setDir(QString const& dir);
    QString dir() const;

    bool cd(QString const& dir);
    void sortItems(int index, bool isDescendingOrder);


    CompressFileInfo::Ptr fileInfo(int index);
    bool isParent(int index) const;
    QString filePath(int index) const;

    bool rm(QString const& filePath);
    bool rm(QStringList const& fileNames);
    bool rename(QString const& oldFileName, QString const& newFileName);
    bool extract(QString const& targetPath, QStringList const& fileNames, bool isWithPath);
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
    int sortIndex;
    bool isDescending;
};

#endif // COMPRESSDIRMODEL_H
