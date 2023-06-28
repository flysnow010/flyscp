#ifndef DIRMODEL_H
#define DIRMODEL_H

#include "treemodel.h"

class DirModel : public TreeModel
{
public:
    explicit DirModel(QObject *parent = nullptr);
    enum IconShowType { None, Standard, All, ALLWithExeAndLink };

    IconShowType iconShowType() const { return iconShowType_; }
    void setIconShowType(IconShowType const& showType) { iconShowType_ = showType; }

    bool isShowIconForFyleSystem() const { return isShowIconForFyleSystem_; }
    void showIconForFyleSystem(bool isShow) { isShowIconForFyleSystem_ = isShow; }
    bool isShowIconForVirtualFolder() const { return isShowIconForVirtualFolder_; }
    void showIconForVirtualFolder(bool isShow) { isShowIconForVirtualFolder_ = isShow; }

    bool isShowOverlayIcon() const { return isShowOverlayIcon_; }
    void showOverlayIcon(bool isShow) { isShowOverlayIcon_ = isShow; }

    bool isShowHidden() const { return isShowHidden_; }
    void showHidden(bool isShow) { isShowHidden_ = isShow; }

    bool isShowSystem() const { return isShowSystem_; }
    void showSystem(bool isShow) { isShowSystem_ = isShow; }

    bool isShowToolTips() const { return isShowToolTips_; }
    void showToolTips(bool isShow) { isShowToolTips_ = isShow; }

    bool isShowParentInRoot() const { return isShowParentInRoot_; }
    void showParentInRoot(bool isShow) { isShowParentInRoot_ = isShow; }

    bool isRenameBaseName() const { return isRenameBaseName_; }
    void setRenameBaseName(bool isShow) { isRenameBaseName_ = isShow; }

    bool dirSortIsByTime() const { return dirSortIsByTime_; }
    void setDirSortByTime(bool isShow) { dirSortIsByTime_ = isShow; }

    int fileCount () const { return fileCount_; }
    int dirCount() const { return dirCount_; }
    qint64 fileSizes() const { return fileSizes_; }

    void setTextColor(QString const& color) { textColor_ = color; }
    QString textColor() const { return textColor_; }

    void setBackground(QString const& color) { backColor_ = color; }
    QString background() const { return backColor_; }

    void setAltColor(QString const& color) { altColor_ = color; }
    QString altColo() const { return altColor_; }

    virtual bool isParent(int index) const = 0;
    virtual bool isDir(int index) const = 0;
    virtual bool isFile(int index) const = 0;
    virtual qint64 fileSize(int index) const = 0;
protected:
    QVariant backColor(const QModelIndex &index) const override;
    QVariant foreColor(const QModelIndex &index) const override;
protected:
    int fileCount_ = 0;
    int dirCount_ = 0;
    qint64 fileSizes_ = 0;
private:
    IconShowType iconShowType_ = All;
    bool isShowIconForFyleSystem_ = true;
    bool isShowIconForVirtualFolder_ = true;
    bool isShowOverlayIcon_ = true;
    bool isShowHidden_ = false;
    bool isShowSystem_ = false;
    bool isShowToolTips_ = true;
    bool isShowParentInRoot_ = false;
    bool isRenameBaseName_ = false;
    bool dirSortIsByTime_ = false;
    QString textColor_ = "#454545";
    QString backColor_ = "#ffffff";
    QString altColor_ = "#f9f9f9";
};

#endif // DIRMODEL_H
