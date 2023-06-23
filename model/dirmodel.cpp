#include "dirmodel.h"
#include <QBrush>
#include <QColor>

DirModel::DirModel(QObject *parent)
    : TreeModel(parent)
    , iconShowType_(All)
    , textColor_("#454545")
    , backColor_("#ffffff")
    , altColor_("#f9f9f9")
{

}

QVariant DirModel::backColor(const QModelIndex &index) const
{
    if((index.row() % 2) == 0)
        return QBrush(QColor(backColor_));
    else
        return QBrush(QColor(altColor_));
}

QVariant DirModel::foreColor(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QBrush(QColor(textColor_));
}
