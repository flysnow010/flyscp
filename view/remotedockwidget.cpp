#include "remotedockwidget.h"
#include "ui_remotedockwidget.h"
#include "model/remotedirmodel.h"

RemoteDockWidget::RemoteDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::RemoteDockWidget)
    , model_(new RemoteDirModel(this))
{
    ui->setupUi(this);
    ui->treeView->setModel(model_);
}

RemoteDockWidget::~RemoteDockWidget()
{
    delete ui;
}
