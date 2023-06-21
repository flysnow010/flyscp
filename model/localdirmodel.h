#ifndef LOCALDIRMODEL_H
#define LOCALDIRMODEL_H

#include "treemodel.h"
#include <QIcon>
#include <QDir>
#include <QFileInfoList>

class LocalDirModel : public TreeModel
{
public:
    explicit LocalDirModel(QObject *parent = nullptr);

    void setDir(QString const& dir);
    QString dir() const;
    void showHidden(bool isShow);
    void showSystem(bool isShow);
    void showToolTips(bool isShow);
    void showParentInRoot(bool isShow);
    void setDirSoryByTime(bool isOn);

    bool mkdirs(QString const& dir);
    bool rename(QString const& original, QString const& newname);
    void refresh();

    bool cd(const QString &dirName);
    void sortItems(int index, bool isDescendingOrder);

    QFileInfo const& fileInfo(int index);
    QString fileName(int index);
    QString filePath(int index);
    QString filePath(QString const& fileName);
    int indexOfFile(QString const& fileName);
    bool isParent(int index) const;
    int fileCount () const { return fileCount_; }
    int dirCount() const { return dirCount_; }
    qint64 fileSizes() const { return fileSizes_; }

    void cancheIcon(QString const& suffix, QIcon const& icon);
protected:
    virtual TreeItem *createRootItem();
    virtual QVariant textAlignment(const QModelIndex &index) const;
    virtual bool editable(const QModelIndex &index) const;
    virtual QVariant headerTextAlignment(int column) const;
    virtual QVariant icon(const QModelIndex &index) const;
    virtual QVariant userData(const QModelIndex &index) const;
    virtual QVariant toolTip(const QModelIndex &index) const;
    virtual void setupModelData(TreeItem *parent);
    virtual QVariant foreColor(const QModelIndex &index) const;
private:
    void modifyFileInfos(QFileInfoList &fileIfos);
    void defaultRefresh();
    QDir::Filters getFilters();
private:
    QDir dir_;
    QFileInfoList fileInfos_;
    QMap<QString, QIcon> iconMap;
    bool isShowHidden_;
    bool isShowSystem_;
    bool isShowToolTips_;
    bool isShowParentInRoot_;
    bool dirSortIsByTime_;
    QIcon dirIcon;
    QIcon backIcon;
    int sortIndex;
    bool isDescending;
    int fileCount_;
    int dirCount_;
    qint64 fileSizes_;
};

#endif // LOCALDIRMODEL_H
