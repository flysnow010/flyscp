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

    void setTextColor(QString const& color) { textColor_ = color; }
    QString textColor() const { return textColor_; }

    void setBackground(QString const& color) { backColor_ = color; }
    QString background() const { return backColor_; }

    void setAltColor(QString const& color) { altColor_ = color; }
    QString altColo() const { return altColor_; }
protected:
    QVariant backColor(const QModelIndex &index) const override;
    QVariant foreColor(const QModelIndex &index) const override;
private:
    IconShowType iconShowType_;
    bool isShowIconForFyleSystem_ = true;
    bool isShowIconForVirtualFolder_ = true;
    bool isShowOverlayIcon_ = true;
    bool isShowHidden_ = false;
    bool isShowSystem_ = false;
    bool isShowToolTips_ = true;
    bool isShowParentInRoot_ = false;
    bool isRenameBaseName_ = false;
    bool dirSortIsByTime_ = false;
    QString textColor_;
    QString backColor_;
    QString altColor_;
};

#endif // DIRMODEL_H
