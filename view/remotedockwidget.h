#ifndef REMOTEDOCKWIDGET_H
#define REMOTEDOCKWIDGET_H

#include <QDockWidget>

namespace Ui {
class RemoteDockWidget;
}
class RemoteDirModel;
class RemoteDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit RemoteDockWidget(QWidget *parent = nullptr);
    ~RemoteDockWidget();

private:
    Ui::RemoteDockWidget *ui;
    RemoteDirModel* model_;
};

#endif // REMOTEDOCKWIDGET_H
