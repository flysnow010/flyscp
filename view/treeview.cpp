#include "treeview.h"

#include <QHeaderView>
#include <QMouseEvent>
#include <QApplication>

TreeView::TreeView(QWidget *parent)
    : QTreeView(parent)
    , mousePress(false)
    , isDrag(false)
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
        isDrag = true;
    }
    if(isDrag)
        return;
    QTreeView::mouseMoveEvent(event);
}

void TreeView::mouseReleaseEvent(QMouseEvent *event)
{
    mousePress = false;
    isDrag = false;
    QTreeView::mouseReleaseEvent(event);
}

void TreeView::dragEnterEvent(QDragEnterEvent * event)
{
    emit dragEnter(event);
}

void TreeView::dragMoveEvent(QDragMoveEvent * event)
{
    emit dragMove(event);
}

void TreeView::dropEvent(QDropEvent * event)
{
    emit drop(event);
}
