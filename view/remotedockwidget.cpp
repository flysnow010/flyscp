#include "remotedockwidget.h"
#include "ui_remotedockwidget.h"
#include "model/remotedirmodel.h"
#include "sftp/sftpsession.h"
#include <QDebug>

#include <QMenu>
#include <QSettings>

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
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)),
                    this, SLOT(customContextMenuRequested(QPoint)));
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

void RemoteDockWidget::customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);
    QMenu menu;
    if(!index.isValid())
    {
        menu.addAction(QIcon(":/image/back.png"), "Parent directory", this, SLOT(parentDirectory()));
        menu.addSeparator();
        menu.addAction("New directory", this, SLOT(makeDirectory()));
        menu.addAction("New file", this, SLOT(newFile()));
        menu.addSeparator();
        menu.addAction("Refresh current folder", this, SLOT(refreshFolder()));
        menu.addAction("Upload to current folder", this, SLOT(upload()));
    }
    else
    {
        ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
        if(!fileInfo)
            return;
        if(fileInfo->isDir())
        {
             menu.addAction("Open", this, SLOT(open()));
             menu.addAction("Download", this, SLOT(download()));
             menu.addSeparator();
             menu.addAction("Delete", this, SLOT(del()));
             menu.addAction("Rename", this, SLOT(rename()));
             menu.addSeparator();
             menu.addAction("Copy file path", this, SLOT(copyFilepath()));
             menu.addAction("Properties", this, SLOT(properties()));
             menu.addAction("Permissions", this, SLOT(permissions()));
        }

        else if(fileInfo->isFile())
        {
            menu.addAction("Open", this, SLOT(open()));
            menu.addAction("Open with...", this, SLOT(openWith()));
            menu.addAction("Download", this, SLOT(download()));
            menu.addSeparator();
            menu.addAction("Delete", this, SLOT(del()));
            menu.addAction("Rename", this, SLOT(rename()));
            menu.addSeparator();
            menu.addAction("Copy file path", this, SLOT(copyFilepath()));
            menu.addAction("Properties", this, SLOT(properties()));
            menu.addAction("Permissions", this, SLOT(permissions()));
        }
    }
    menu.exec(QCursor::pos());
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

void RemoteDockWidget::parentDirectory()
{

}

void RemoteDockWidget::makeDirectory()
{

}

void RemoteDockWidget::newFile()
{

}

void RemoteDockWidget::refreshFolder()
{

}

void RemoteDockWidget::upload()
{

}

void RemoteDockWidget::open()
{

}

void RemoteDockWidget::openWith()
{

}

void RemoteDockWidget::download()
{

}

void RemoteDockWidget::del(){

}

void RemoteDockWidget::rename(){

}

void RemoteDockWidget::copyFilepath(){

}

void RemoteDockWidget::properties()
{

}

void RemoteDockWidget::permissions()
{

}
