#ifndef REMOTEDOCKWIDGET_H
#define REMOTEDOCKWIDGET_H

#include <QDockWidget>

namespace Ui {
class RemoteDockWidget;
}

class RemoteDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit RemoteDockWidget(QWidget *parent = nullptr);
    ~RemoteDockWidget();

private:
    Ui::RemoteDockWidget *ui;
};

#endif // REMOTEDOCKWIDGET_H
