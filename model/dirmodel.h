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
    void setShowIconForFyleSystem(bool isShow) { isShowIconForFyleSystem_ = isShow; }
    bool isShowIconForVirtualFolder() const { return isShowIconForVirtualFolder_; }
    void setShowIconForVirtualFolder(bool isShow) { isShowIconForVirtualFolder_ = isShow; }

    bool isShowOverlayIcon() const { return isShowOverlayIcon_; }
    void setShowOverlayIcon(bool isShow) { isShowOverlayIcon_ = isShow; }

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
    bool isShowIconForFyleSystem_;
    bool isShowIconForVirtualFolder_;
    bool isShowOverlayIcon_;
    QString textColor_;
    QString backColor_;
    QString altColor_;
};

#endif // DIRMODEL_H
