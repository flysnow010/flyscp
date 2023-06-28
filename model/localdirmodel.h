#ifndef LOCALDIRMODEL_H
#define LOCALDIRMODEL_H

#include "dirmodel.h"
#include <QIcon>
#include <QDir>
#include <QFileInfoList>

class LocalDirModel : public DirModel
{
public:
    explicit LocalDirModel(QObject *parent = nullptr);

    void setDir(QString const& dir);
    QString dir() const;

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
    void modifyFileInfos(QFileInfoList &fileIfos);
    void defaultRefresh();
    QDir::Filters getFilters();
private:
    QDir dir_;
    QFileInfoList fileInfos_;
    QMap<QString, QIcon> iconMap;
    QIcon dirIcon;
    QIcon backIcon;
    int sortIndex;
    bool isDescending;
};

#endif // LOCALDIRMODEL_H
