#include "remotedockwidget.h"
#include "ui_remotedockwidget.h"
#include "titlebarwidget.h"
#include "model/remotedirmodel.h"
#include "sftp/sftpsession.h"
#include "dialog/propertydialog.h"
#include "dialog/permissionsdialog.h"
#include "core/clipboard.h"
#include "core/filemanager.h"
#include "core/winshell.h"
#include "core/dirhistory.h"
#include "core/dirfavorite.h"
#include "core/filename.h"
#include "core/remotefiletransfer.h"
#include "core/sftpfilemanager.h" //for test
#include "util/utils.h"
#include "dialog/fileprogressdialog.h"

#include <QDebug>

#include <QMenu>
#include <QSettings>
#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>
#include <QMimeData>

RemoteDockWidget::RemoteDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::RemoteDockWidget)
    , model_(new RemoteDirModel(this))
    , titleBarWidget(new TitleBarWidget(false))
    , sftp(new SFtpSession(this))
    , dirFavorite(new DirFavorite())
    , dirHistory(new DirHistory())
{
    ui->setupUi(this);
    ui->treeView->setModel(model_);
    setTitleBarWidget(titleBarWidget);
    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)),
                    this, SLOT(viewClick(QModelIndex)));
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)),
                    this, SLOT(customContextMenuRequested(QPoint)));
    connect(ui->treeView->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
    connect(ui->treeView, SIGNAL(prepareDrag(QPoint)),
                    this, SLOT(beginDragFile(QPoint)));
    connect(ui->treeView, SIGNAL(dragEnter(QDragEnterEvent*)),
                    this, SLOT(dragEnter(QDragEnterEvent*)));
    connect(ui->treeView, SIGNAL(dragMove(QDragMoveEvent*)),
                    this, SLOT(dragMove(QDragMoveEvent*)));
    connect(ui->treeView, SIGNAL(drop(QDropEvent*)),
                    this, SLOT(drop(QDropEvent*)));
    connect(sftp, &SFtpSession::connected, this, &RemoteDockWidget::connected);
    connect(sftp, &SFtpSession::unconnected, this, &RemoteDockWidget::unconnected);
    connect(sftp, &SFtpSession::connectionError, this, &RemoteDockWidget::connectionError);

    connect(titleBarWidget, SIGNAL(libDirButtonClicked()),
                    this, SLOT(libDirContextMenu()));
    connect(titleBarWidget, SIGNAL(favoritesDirButtonCLicked()),
                    this, SLOT(favoritesDirContextMenu()));
    connect(titleBarWidget, SIGNAL(historyDirButtonClicked()),
                    this, SLOT(historyDirContextMenu()));
    connect(titleBarWidget, &TitleBarWidget::dirSelected, this, [&](QString const& dir)
    {
        setDir(dir);
    });
}

RemoteDockWidget::~RemoteDockWidget()
{
    saveSettings();
    sftp->stop();
    delete dirFavorite;
    delete dirHistory;
    delete ui;
}

void RemoteDockWidget::setDir(QString const& dir, QString const& caption, bool  isNavigation)
{
    model_->setDir(sftp->dir(dir.toStdString()));
    updateCurrentDir(model_->dirName(), caption, isNavigation);
}

QString RemoteDockWidget::dir() const
{
    return model_->dirName();
}

void RemoteDockWidget::cd(QString const& dir)
{
    if(dir == "..")
        parentDirectory();
    else
    {

        ssh::FileInfoPtr fileInfo = model_->fileInfo(dir);
        if(!fileInfo)
            return;

        if(fileInfo->is_dir())
            openDir(fileInfo);
    }
}

QString RemoteDockWidget::home() const
{
    return QString::fromStdString(sftp->homeDir());
}

QString RemoteDockWidget::root() const
{
    return QString("/");
}

void RemoteDockWidget::libDirContextMenu()
{

}

void RemoteDockWidget::favoritesDirContextMenu()
{
    QMenu menu;

    QString currentFileName = this->dir();
    QList<FavoriteItem> items = dirFavorite->favoriteItems();
    bool isCurrent = false;
    foreach(auto const& item, items)
    {
        QAction* action = menu.addAction(item.caption, this, [&](bool)
        {
            this->setDir(item.fileName);
        }
        );
        if(currentFileName == item.fileName)
        {
            action->setCheckable(true);
            action->setChecked(true);
            isCurrent = true;
        }
    }
    menu.addSeparator();
    FavoriteItem item;
    item.caption = QFileInfo(currentFileName).fileName();
    item.fileName = currentFileName;

    if(isCurrent)
        menu.addAction("Remove current folder", this, [&](bool){
            dirFavorite->removeItem(item);
        });
    else
        menu.addAction("Add current folder", this, [&](bool){
            QString caption = Utils::getText("Cation of new menu", item.caption);
            if(!caption.isEmpty())
                item.caption = caption;
            dirFavorite->addItem(item);
    });
    menu.addAction("Settings");
    menu.exec(QCursor::pos());
}

void RemoteDockWidget::historyDirContextMenu()
{
    QStringList const& dirNames = dirHistory->dirs();
    QMenu menu;
    QString currentDir = this->dir();

    foreach(auto const& dirName, dirNames)
    {
        QAction* action = menu.addAction(dirName, this, [&](bool){
            this->setDir(dirName);
        });

        if(dirName == currentDir)
        {
            action->setCheckable(true);
            action->setChecked(true);
        }
    }
    menu.exec(QCursor::pos());
}

void RemoteDockWidget::start(SSHSettings const& settings)
{
    name_ = QString("%1@%2").arg(settings.userName, settings.hostName);
    sftp->start(settings);
    loadSettings();
}

void RemoteDockWidget::saveSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup(name_);
    QHeaderView *headerView = ui->treeView->header();
    settings.setValue("DirName", windowTitle());
    settings.beginWriteArray("sectionSizes", headerView->count());
    for(int i = 0; i < headerView->count(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("sectionSize", headerView->sectionSize(i));
    }
    settings.endArray();
    settings.endGroup();
}

void RemoteDockWidget::loadSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup(name_);
    QHeaderView *headerView = ui->treeView->header();
    int size = settings.beginReadArray("sectionSizes");
    for(int i = 0; i < size && i < headerView->count(); i++)
    {
        settings.setArrayIndex(i);
        settings.value("sectionSize").toInt();
        headerView->resizeSection(i, settings.value("sectionSize").toInt());
    }
    settings.endArray();
    settings.endGroup();
}


void RemoteDockWidget::viewClick(QModelIndex const& index)
{
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo)
        return;
    if(fileInfo->is_dir())
        openDir(fileInfo);
    else if(fileInfo->is_file())
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
        menu.addAction(QIcon(":/image/back.png"), tr("Parent directory"), this, SLOT(parentDirectory()));
        menu.addSeparator();
        menu.addAction(tr("New directory"), this, SLOT(makeDirectory()));
        menu.addAction(tr("New file"), this, SLOT(newFile()));
        menu.addSeparator();
        menu.addAction(tr("Refresh current folder"), this, SLOT(refreshFolder()));
        menu.addAction(tr("Upload to current folder"), this, SLOT(upload()));
    }
    else
    {
        ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
        if(!fileInfo)
            return;

        menu.addAction(tr("Open"), this, SLOT(open()));
        if(fileInfo->is_file())
            menu.addAction(tr("Open with..."), this, SLOT(openWith()));
        menu.addAction(tr("Download"), this, SLOT(download()));
        menu.addSeparator();
        menu.addAction(tr("Delete"), this, SLOT(deleteFiles()));
        menu.addAction(tr("Rename"), this, SLOT(rename()));
        menu.addSeparator();
        menu.addAction(tr("Copy file path"), this, SLOT(copyFilepath()));
        menu.addAction(tr("Permissions"), this, SLOT(permissions()));
        menu.addAction(tr("Properties"), this, SLOT(properties()));
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

void RemoteDockWidget::beginDragFile(QPoint const& point)
{
    QModelIndex index = ui->treeView->indexAt(point);
    if(!index.isValid())
        return;
//    QDrag *drag = new QDrag(ui->treeView);
//    QStringList fileNames = selectedileNames("file:///");
//    QMimeData* mimeData = WinShell::dropMimeData(fileNames);
//    drag->setMimeData(mimeData);
//    drag->setPixmap(QPixmap(":/image/copy.png"));
//    drag->exec();
}

void RemoteDockWidget::dragEnter(QDragEnterEvent * event)
{
    QMimeData const* mimeData = event->mimeData();
    if(mimeData)
        event->acceptProposedAction();
}

void RemoteDockWidget::dragMove(QDragMoveEvent * event)
{
    QModelIndex index = ui->treeView->indexAt(event->pos());
    bool isSelf = (event->source() == ui->treeView);
    if(index.isValid())
    {
        if(isSelf && model_->fileInfo(index.row())->is_file())
            event->ignore();
        else
            event->acceptProposedAction();
    }
    else
    {
        if(isSelf)
            event->ignore();
        else
            event->acceptProposedAction();
    }
}

void RemoteDockWidget::drop(QDropEvent * event)
{
    QModelIndex index = ui->treeView->indexAt(event->pos());
    QString filePath;
    if(!index.isValid())
        filePath = model_->dirName();
    else
    {
        if(model_->fileInfo(index.row())->is_file())
            filePath = model_->dirName();
        else
            filePath = QString::fromStdString(model_->filePath(index.row()));
    }

    QMimeData const* mimeData = event->mimeData();
    if(!mimeData)
        return;

    QStringList fileNames = mimeData->text().split("\n");
    fileTransfer(FileNames::GetFileNames(fileNames, filePath, "file:///"), false);
    model_->refresh();
}

void RemoteDockWidget::connected()
{
    std::string homeDir = sftp->homeDir();
    if(homeDir.empty())
        model_->setDir(sftp->home());
    else
        model_->setDir(sftp->dir(homeDir));
    QString dir = model_->dirName();
    updateCurrentDir(dir);
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
    ssh::FileInfoPtr fileInfo = model_->fileInfo(0);
    if(!fileInfo  || !fileInfo->is_parent())
        return;

    std::string filePath = model_->parentPath();
    model_->setDir(sftp->dir(filePath));
    updateCurrentDir(model_->dirName());
}

void RemoteDockWidget::makeDirectory()
{
    QString path = Utils::getText(tr("New folder"));
    if(path.isEmpty())
        return;
    if(model_->mkdir(path.toStdString()))
        model_->refresh();
}

void RemoteDockWidget::newFile()
{
    QString fileName = Utils::getText(tr("New file"));
    if(fileName.isEmpty())
        return;
    if(model_->mkFile(fileName.toStdString()))
        model_->refresh();
}

void RemoteDockWidget::refreshFolder()
{
    model_->refresh();
}

void RemoteDockWidget::upload()
{
    QString fileName = QFileDialog::getOpenFileName(this, QApplication::applicationName());
    if(!fileName.isEmpty() && upload(fileName))
        model_->refresh();
}

void RemoteDockWidget::open()
{
    QModelIndex index = ui->treeView->currentIndex();
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo)
        return;
    if(fileInfo->is_dir())
        openDir(fileInfo);
    else
    {
        QString fileName = download(fileInfo, Utils::tempPath());
        if(!fileName.isEmpty())
            WinShell::Open(fileName);
    }
}

void RemoteDockWidget::openWith()
{
    QModelIndex index = ui->treeView->currentIndex();
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo)
        return;

    QString fileName = download(fileInfo, Utils::tempPath());
    if(!fileName.isEmpty())
        WinShell::OpenWith(fileName);
}

void RemoteDockWidget::download()
{
    QString filePath = QFileDialog::getExistingDirectory(this, QApplication::applicationName());
    if(filePath.isEmpty())
        return;

    QModelIndex index = ui->treeView->currentIndex();
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo)
        return;

    download(fileInfo, QDir(filePath));
}

void RemoteDockWidget::deleteDir()
{
    QModelIndex index = ui->treeView->currentIndex();
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo)
        return;
    model_->rmdir(fileInfo->basename());
    model_->refresh();
}

void RemoteDockWidget::deleteFile()
{
    QModelIndex index = ui->treeView->currentIndex();
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo)
        return;

    model_->rmFile(fileInfo->name());
    model_->refresh();
}

void RemoteDockWidget::deleteFiles()
{
    qDebug() << selectedileNames() << model_->dirName();
}

QStringList RemoteDockWidget::selectedileNames()
{
    QModelIndexList indexs = ui->treeView->selectionModel()->selectedRows(0);
    QStringList names;
    for(int i = 0; i < indexs.size(); i++)
    {
        names << QString::fromStdString(model_->filePath(indexs[i].row()));
    }
    return names;
}

void RemoteDockWidget::rename()
{
    QModelIndex index = ui->treeView->currentIndex();
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo)
        return;

    QString fileName = Utils::getText(tr("New filename"), QString::fromStdString(fileInfo->name()));
    if(fileName.isEmpty())
        return;

    model_->rename(fileInfo->name(), fileName.toStdString());
    model_->refresh();
}

void RemoteDockWidget::copyFilepath()
{
    QModelIndex index = ui->treeView->currentIndex();
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo)
        return;

    std::string filepath = model_->filePath(fileInfo->name());
    ClipBoard::copy(QString::fromStdString(filepath));
}

void RemoteDockWidget::properties()
{
    QModelIndex index = ui->treeView->currentIndex();
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo)
        return;
    PropertyDialog dialog(this);
    dialog.setFileInfo(fileInfo);
    dialog.exec();
}

void RemoteDockWidget::permissions()
{
    QModelIndex index = ui->treeView->currentIndex();
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo)
        return;
    PermissionsDialog dialog(this);
    dialog.setFileName(QString::fromStdString(fileInfo->name()));
    dialog.setPermissions(fileInfo->permissions(), fileInfo->is_dir());
    if(dialog.exec() == QDialog::Accepted)
    {
        model_->chmod(fileInfo->name(), dialog.permissions());
        model_->refresh();
    }
}

void RemoteDockWidget::openDir(ssh::FileInfoPtr const& fileInfo)
{
    if(std::string(fileInfo->owner()) != sftp->userName()
            && !fileInfo->other_is_only_read())
    {
        QMessageBox::warning(this, QApplication::applicationName(), "Permission denied");
        return;
    }

    std::string filePath;
    if(fileInfo->is_parent())
        filePath = model_->parentPath();
    else
        filePath = model_->filePath(fileInfo->name());
    model_->setDir(sftp->dir(filePath));
    QString dir = model_->dirName();
    updateCurrentDir(dir);
}

QString RemoteDockWidget::download(ssh::FileInfoPtr const& fileInfo, QDir const& dstDir)
{
    ssh::File::Ptr remotefile = sftp->openForRead(model_->filePath(fileInfo->name()).c_str());
    if(!remotefile)
        return QString();

     QString fileName =  dstDir.filePath(QString::fromStdString(fileInfo->name()));
     QFile file(fileName);
     if(!file.open(QIODevice::WriteOnly))
         return QString();
     uint64_t filesize = fileInfo->size();
     while(filesize > 0)
     {
         char data[1024];
         ssize_t size = remotefile->read(data, sizeof(data));
         if(size <= 0)
             break;
         filesize -= size;
         file.write(data, size);
     }
     if(filesize > 0)
         return QString();
     return fileName;
}

bool RemoteDockWidget::upload(QString const& fileName)
{
    SFtpFileManager fileManger(sftp);
    fileManger.uploadFiles(FileNames::GetFileNames(fileName, model_->dirName()));
    return true;
}

void RemoteDockWidget::fileTransfer(FileNames const& fileNames, bool isDowload)
{
    RemoteFileTransfer transfer(new SFtpFileManager(sftp));
    FileProgressDialog dialog(this);

    connect(&transfer, &RemoteFileTransfer::totalProgress, &dialog, &FileProgressDialog::totalProgress);
    connect(&transfer, &RemoteFileTransfer::fileProgress, &dialog, &FileProgressDialog::fileProgress);
    connect(&transfer, &RemoteFileTransfer::finished, &dialog, &FileProgressDialog::finished);
    connect(&transfer, &RemoteFileTransfer::error, &dialog, &FileProgressDialog::error);
    dialog.setModal(true);
    dialog.show();
    if(isDowload)
    {
        dialog.setWindowTitle("DownloadFiles");
        transfer.downloadFiles(fileNames);
    }
    else
    {
        dialog.setWindowTitle("UploadFiles");
        transfer.uploadFiles(fileNames);
    }
    while(!dialog.isFinished())
    {
        if(dialog.isCancel())
        {
            transfer.cancel();
            while(!dialog.isFinished())
                QApplication::processEvents();
        }
        QApplication::processEvents();
    }
}

void RemoteDockWidget::updateCurrentDir(QString const& dir, QString const& caption, bool  isNavigation)
{
    if(caption.isEmpty())
        titleBarWidget->setTitle(dir);
    else
        titleBarWidget->setTitle(caption);
    if(!isNavigation)
        dirHistory->add(dir);
}
