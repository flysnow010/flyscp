#include "dirmodel.h"
#include <QBrush>
#include <QColor>

DirModel::DirModel(QObject *parent)
    : TreeModel(parent)
{
}

QVariant DirModel::backColor(const QModelIndex &index) const
{
    if((index.row() % 2) == 0)
        return QBrush(QColor(altColor_));
    else
        return QBrush(QColor(backColor_));
}

QVariant DirModel::foreColor(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QBrush(QColor(textColor_));
}
