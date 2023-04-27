#include "treeview.h"
#include <QHeaderView>
#include <QMouseEvent>
#include <QApplication>

TreeView::TreeView(QWidget *parent)
    : QTreeView(parent)
    , mousePress(false)
{
    setMouseTracking(true);
    setAcceptDrops(true);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    header()->setStretchLastSection(false);
}

void TreeView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        pressPoint = event->pos();
        mousePress = true;
    }
    QTreeView::mousePressEvent(event);
}

void TreeView::mouseMoveEvent(QMouseEvent *event)
{
    if(mousePress)
    {
        int distance = (event->pos() - pressPoint).manhattanLength();
        if(distance >= QApplication::startDragDistance())
        {
            mousePress = false;
            emit prepareDrag(pressPoint);

        }
    }
    QTreeView::mouseMoveEvent(event);
}

void TreeView::mouseReleaseEvent(QMouseEvent *event)
{
    mousePress = false;
    QTreeView::mouseReleaseEvent(event);
}

void TreeView::dragEnterEvent(QDragEnterEvent * event)
{
//    const ClipMimeData* clipData = dynamic_cast<const ClipMimeData*>(event->mimeData());
//    if(clipData)
//    {
//        event->acceptProposedAction();
//        dragObjectType_ = ClipObjectType;
//        return;
//    }
}

void TreeView::dragMoveEvent(QDragMoveEvent * event)
{
//    QModelIndex index = indexAt(event->pos());
//    if(dragObjectType_ == ClipObjectType
//        || dragObjectType_ == FilterObjectType)
//    {
//        if(index.isValid() && index.row() %2 == 0)
//            event->acceptProposedAction();
//        else
//            event->ignore();
//    }
//    else if(dragObjectType_ == TranstionObjectType)
//    {
//        if(index.isValid() && index.row()%2 != 0)
//            event->acceptProposedAction();
//        else
//            event->ignore();
//    }
}

void TreeView::dropEvent(QDropEvent * event)
{
//    QModelIndex index = indexAt(event->pos());
//    if(dragObjectType_ == ClipObjectType)
//    {
//        const ClipMimeData* clipData = dynamic_cast<const ClipMimeData*>(event->mimeData());
//        if(index.isValid() && index.row() % 2 == 0)
//        {
//            ClipPtr clip = clipData->clip();
//            emit insertClip(clip, index.row());
//        }
//    }
}
