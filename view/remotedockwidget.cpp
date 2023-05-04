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
    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)),
                    this, SLOT(viewClick(QModelIndex)));
    connect(ui->treeView->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
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

void RemoteDockWidget::viewClick(QModelIndex const& index)
{
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo)
        return;
    if(fileInfo->isDir())
    {
        std::string filePath;
        if(fileInfo->isParent())
            filePath = model_->parentPath();
        else
            filePath = model_->filePath(fileInfo->name());
        model_->setDir(sftp->dir(filePath));
        setWindowTitle(model_->dirName());
    }
    else if(fileInfo->isFile())
    {
        ;
    }
}

void RemoteDockWidget::sortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    if(order == Qt::SortOrder::AscendingOrder)
        model_->sortItems(logicalIndex, false);
    else
        model_->sortItems(logicalIndex, true);
}

void RemoteDockWidget::connected()
{
    std::string homeDir = sftp->homeDir();
    if(homeDir.empty())
        model_->setDir(sftp->home());
    else
        model_->setDir(sftp->dir(homeDir));
    setWindowTitle(model_->dirName());
}

void RemoteDockWidget::unconnected()
{
    qDebug() << "sftp is disconnected";
}

void RemoteDockWidget::connectionError(QString const& error)
{
    qDebug() << "error: " << error;
}
