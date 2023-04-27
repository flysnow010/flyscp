#ifndef LOCALFILEMODEL_H
#define LOCALFILEMODEL_H

#include "treemodel.h"
#include <QIcon>
#include <QDir>
#include <QFileInfoList>

class LocalDirModel : public TreeModel
{
public:
    explicit LocalDirModel(QObject *parent = nullptr);

    void setDir(QString const& dir);
    QString dir();
    void update();

    bool cd(const QString &dirName);
    void sort(int index, bool isDescendingOrder);

    QString fileName(int index);
    QString filePath(int index);
    QString filePath(QString const& fileName);
protected:
    virtual TreeItem *createRootItem();
    virtual QVariant textAlignment(int column) const;
    virtual QVariant icon(const QModelIndex &index) const;
    virtual QVariant userData(const QModelIndex &index) const;
    virtual void setupModelData(TreeItem *parent);
    virtual QVariant foreColor(const QModelIndex &index) const;
private:
    void modifyFileInfos(QFileInfoList &fileIfos);
private:
    QDir dir_;
    QFileInfoList fileInfos_;
    QMap<QString, QIcon> iconMap;
    QIcon dirIcon;
    QIcon backIcon;
};

#endif // LOCALFILEMODEL_H
