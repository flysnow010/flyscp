#include "remotedockwidget.h"
#include "ui_remotedockwidget.h"

RemoteDockWidget::RemoteDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::RemoteDockWidget)
{
    ui->setupUi(this);
}

RemoteDockWidget::~RemoteDockWidget()
{
    delete ui;
}
