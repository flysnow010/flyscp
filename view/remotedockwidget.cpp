#include "remotedockwidget.h"
#include "ui_remotedockwidget.h"
#include "model/remotedirmodel.h"
#include "sftp/sftpsession.h"
#include <QDebug>

RemoteDockWidget::RemoteDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::RemoteDockWidget)
    , model_(new RemoteDirModel(this))
    , sftp(new SFtpSession(this))
{
    ui->setupUi(this);
    ui->treeView->setModel(model_);
    connect(sftp, &SFtpSession::connected, this, &RemoteDockWidget::connected);
    connect(sftp, &SFtpSession::unconnected, this, &RemoteDockWidget::unconnected);
    connect(sftp, &SFtpSession::connectionError, this, &RemoteDockWidget::connectionError);
}

RemoteDockWidget::~RemoteDockWidget()
{
    sftp->stop();
    delete ui;
}

void RemoteDockWidget::start(SSHSettings const& settings)
{
    sftp->start(settings);
}

void RemoteDockWidget::connected()
{
    qDebug() << "sftp is connected";
    model_->setDir(sftp->home());
}

void RemoteDockWidget::unconnected()
{
    qDebug() << "sftp is disconnected";
}

void RemoteDockWidget::connectionError(QString const& error)
{
    qDebug() << "error: " << error;
}
