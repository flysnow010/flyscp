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
#include "core/optionsmanager.h"
#include "core/remotefiletransfer.h"
#include "core/sftpfilemanager.h"
#include "util/utils.h"
#include "dialog/fileprogressdialog.h"

#include <QDebug>

#include <QMenu>
#include <QSettings>
#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>
#include <QDrag>
#include <QMimeData>
#include <QUuid>

RemoteDockWidget::RemoteDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::RemoteDockWidget)
    , model_(new RemoteDirModel(this))
    , titleBarWidget(new TitleBarWidget(false))
    , hideBarWidget(new QWidget(this))
    , sftp(new SFtpSession(this))
    , dirFavorite(new DirFavorite())
    , dirHistory(new DirHistory())
    , remoteID_(QUuid::createUuid().toString())
{
    ui->setupUi(this);
    ui->treeView->setModel(model_);
    ui->treeView->installEventFilter(this);

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
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [=]()
    {
            emit statusTextChanged(getStatusText());
    });

    connect(sftp, &SFtpSession::connected,
            this, &RemoteDockWidget::connected);
    connect(sftp, &SFtpSession::unconnected,
            this, &RemoteDockWidget::unconnected);
    connect(sftp, &SFtpSession::connectionError,
            this, &RemoteDockWidget::connectionError);

    connect(titleBarWidget, SIGNAL(libDirButtonClicked()),
                    this, SLOT(libDirContextMenu()));
    connect(titleBarWidget, SIGNAL(favoritesDirButtonCLicked()),
                    this, SLOT(favoritesDirContextMenu()));
    connect(titleBarWidget, SIGNAL(historyDirButtonClicked()),
                    this, SLOT(historyDirContextMenu()));
    connect(titleBarWidget, &TitleBarWidget::actived, this, [=](){
        emit actived(dir());
        setActived(true);
    });
    connect(titleBarWidget, &TitleBarWidget::dirSelected,
            this, [&](QString const& dir)
    {
        setDir(dir);
    });
    applyOptions();
}

RemoteDockWidget::~RemoteDockWidget()
{
    saveSettings();
    sftp->stop();
    delete titleBarWidget;
    delete dirFavorite;
    delete dirHistory;
    delete ui;
}

void RemoteDockWidget::setDir(QString const& dir,
                              QString const& caption,
                              bool  isNavigation)
{
    model_->setDir(sftp->dir(dir.toStdString()));
    updateCurrentDir(model_->dirName(), caption, isNavigation);
    if(!isActived())
    {
        emit actived(this->dir());
        setActived(true);
    }
}

QString RemoteDockWidget::dir() const
{
    return model_->dirName();
}

QString RemoteDockWidget::findDir(QString const& prefix) const
{
    return dirHistory->find(prefix);
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

void RemoteDockWidget::preDir()
{
    QString newDir = dirHistory->pre(dir());
    if(!newDir.isEmpty())
        setDir(newDir, QString(), true);
}

void RemoteDockWidget::nextDir()
{
    QString newDir = dirHistory->next(dir());
    if(!newDir.isEmpty())
        setDir(newDir, QString(), true);
}

QString RemoteDockWidget::home() const
{
    return QString::fromStdString(sftp->homeDir());
}

QString RemoteDockWidget::root() const
{
    return QString("/");
}

void RemoteDockWidget::refresh()
{
    model_->refresh();
}

void RemoteDockWidget::showHeader(bool isShow)
{
    ui->treeView->setHeaderHidden(!isShow);
}

void RemoteDockWidget::showCurrentDir(bool isShow)
{
    titleBarWidget->setVisible(isShow);
    if(!isShow)
    {
        setTitleBarWidget(hideBarWidget);
        titleBarWidget->showLibDirButton(false);
    }
    else
        setTitleBarWidget(titleBarWidget);
}

void RemoteDockWidget::showDeskNavigationButton(bool isShow)
{
    titleBarWidget->showLibDirButton(isShow);
}

void RemoteDockWidget::showFavoriteButton(bool isShow)
{
    titleBarWidget->showFavoriteButton(isShow);
}

void RemoteDockWidget::showHistoryButton(bool isShow)
{
    titleBarWidget->showHistoryButton(isShow);
}

void RemoteDockWidget::libDirContextMenu()
{
}

void RemoteDockWidget::showHiddenAndSystem(bool isShow)
{
    model_->showHidden(isShow);
    model_->showSystem(isShow);
}

void RemoteDockWidget::showToolTips(bool isShow)
{
    model_->showToolTips(isShow);
}

void RemoteDockWidget::setDirSoryByTime(bool isOn)
{
    model_->setDirSortByTime(isOn);
}

void RemoteDockWidget::setRenameFileName(bool isOn)
{
    model_->setRenameBaseName(isOn);
}

void RemoteDockWidget::showAllIconWithExeAndLink(bool isShow)
{
    if(isShow)
        model_->setIconShowType(DirModel::ALLWithExeAndLink);
}

void RemoteDockWidget::showAllIcon(bool isShow)
{
    if(isShow)
        model_->setIconShowType(DirModel::All);
}

void RemoteDockWidget::showStandardIcon(bool isShow)
{
    if(isShow)
        model_->setIconShowType(DirModel::Standard);
}

void RemoteDockWidget::showNoneIcon(bool isShow)
{
    if(isShow)
        model_->setIconShowType(DirModel::None);
}

void RemoteDockWidget::showIconForFyleSystem(bool isShow)
{
    model_->showIconForFyleSystem(isShow);
}

void RemoteDockWidget::showIconForVirtualFolder(bool isShow)
{
    model_->showIconForVirtualFolder(isShow);
}

void RemoteDockWidget::showOverlayIcon(bool isShow)
{
    model_->showOverlayIcon(isShow);
}

void RemoteDockWidget::setItemColor(QString const& fore,
                  QString const& back,
                  QString const&alternate)
{
    model_->setTextColor(fore);
    model_->setBackground(back);
    model_->setAltColor(alternate);
}

void RemoteDockWidget::setItemSelectedColor(QString const& back,
                                            QString const& mark,
                                            QString const&cursor)
{
    ui->treeView->setStyleSheet(QString("QTreeView{ background: %1;}"
                                        "QTreeView::item:selected:active:first{"
                                        "border: 1px ridge  %2;"
                                        "border-right-width: 0px;"
                                        "color: %4;"
                                        "background: %3;}"
                                        "QTreeView::item:selected:active{"
                                        "border: 1px ridge  %2;"
                                        "border-left-width: 0px;"
                                        "border-right-width: 0px;"
                                        "color: %4;"
                                        "background: %3;}"
                                        "QTreeView::item:selected:active:last{"
                                        "border: 1px ridge  %2;"
                                        "border-left-width: 0px;"
                                        "color: %4;"
                                        "background: %3;}"
                                        )
                                .arg(back, cursor, mark, model_->textColor()));
}

void RemoteDockWidget::fileIconSize(int size)
{
    ui->treeView->setIconSize(QSize(size, size));
}

void RemoteDockWidget::fileFont(QFont const& font)
{
    ui->treeView->setFont(font);
    if(ui->treeView->header())
        ui->treeView->header()->setFont(font);
}


void RemoteDockWidget::showParentInRoot(bool isShow)
{
    model_->showParentInRoot(isShow);
}

void RemoteDockWidget::favoritesDirContextMenu()
{
    QString currentFileName = this->dir();
    QList<FavoriteItem> items = dirFavorite->favoriteItems();
    QMenu menu;

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
        menu.addAction(tr("Remove Current Folder"), this, [&](bool){
            dirFavorite->removeItem(item);
        });
    else
        menu.addAction(tr("Add Current Folder"), this, [&](bool){
            QString caption = Utils::getText(tr("Caption of New Menu"), item.caption);
            if(!caption.isEmpty())
                item.caption = caption;
            dirFavorite->addItem(item);
    });
    menu.addAction(tr("Settings"));
    menu.exec(QCursor::pos());
}

void RemoteDockWidget::historyDirContextMenu()
{
    QStringList const& dirNames = dirHistory->dirs();
    QString currentDir = this->dir();
    QMenu menu;

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

void RemoteDockWidget::execCommand(QString const& command)
{
    QStringList args = command.split(" ");
    if(args.size() < 2)
    {
        ;
    }
    else
    {
        if(args[0] == "cd")
            cd(args[1]);
    }
}

void RemoteDockWidget::viewFile()
{
    QModelIndex index = ui->treeView->currentIndex();
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo || fileInfo->is_dir())
        return;
    QString fileName = download(fileInfo, Utils::tempDir());
    if(fileName.isEmpty())
        return;

    FileNames::MakeFileNameAsParams(fileName);
    WinShell::Exec(Utils::viewApp(), fileName);
}

void RemoteDockWidget::deleteFiles()
{
    QStringList fileNames = selectedileNames();
    QString tipText;
    if(fileNames.size() > 1)
        tipText = QString(tr("Are you sure you want to delete %1 files or folders?\n\n%2"))
                .arg(fileNames.size())
                .arg(fileNames.join("\n"));
    else
    {
        QModelIndex index = ui->treeView->currentIndex();
        QString type = model_->isDir(index.row()) ? tr("folder") : tr("file");
        tipText = QString(tr("Are you sure you want to delete the %1 %2?"))
                .arg(type, fileNames.first());
    }
    if(tipText.isEmpty())
    {
        Utils::warring(tr("No files or folders selected!"));
        return;
    }

    if(Utils::question(tipText))
        delFiles();
}

void RemoteDockWidget::setActived(bool isActived)
{
    titleBarWidget->setActived(isActived);
}

bool RemoteDockWidget::isActived() const
{
    return titleBarWidget->isActived();
}

void RemoteDockWidget::retranslateUi()
{
    ui->retranslateUi(this);
    model_->reset();
    emit statusTextChanged(getStatusText());
}

void RemoteDockWidget::saveSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());

    settings.beginGroup(name_);
    settings.setValue("DirName", windowTitle());

    QHeaderView *headerView = ui->treeView->header();
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

bool RemoteDockWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::FocusIn)
    {
        emit actived(dir());
        setActived(true);
    }
    return QDockWidget::eventFilter(obj, event);
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
        menu.addAction(QIcon(":/image/back.png"), tr("Parent Directory"),
                       this, SLOT(parentDirectory()));
        menu.addSeparator();
        menu.addAction(tr("New Folder"), this, [=](){ newFolder(); });
        menu.addAction(tr("New Txt File"), this, [=](){ newTxtFile(); });
        menu.addSeparator();
        menu.addAction(tr("Refresh Current Folder"), this, [=](){ refresh(); });
        menu.addAction(tr("Upload To Current Folder"), this, SLOT(upload()));
    }
    else
    {
        ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
        if(!fileInfo)
            return;

        menu.addAction(tr("Open"), this, SLOT(open()));
        if(fileInfo->is_file())
            menu.addAction(tr("Open With..."), this, SLOT(openWith()));
        menu.addAction(tr("Download"), this, SLOT(download()));
        menu.addSeparator();
        menu.addAction(tr("Delete"), this, SLOT(delFiles()));
        menu.addAction(tr("Rename"), this, SLOT(rename()));
        menu.addSeparator();
        menu.addAction(tr("Copy File Path"), this, SLOT(copyFilepath()));
        menu.addAction(tr("Permissions"), this, SLOT(permissions()));
        menu.addAction(tr("Properties"), this, SLOT(properties()));
    }
    menu.exec(QCursor::pos());
}

void RemoteDockWidget::sortIndicatorChanged(int logicalIndex,
                                            Qt::SortOrder order)
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

    QDrag *drag = new QDrag(ui->treeView);
    QStringList fileNames = selectedileNames();
    QMimeData* mimeData = new QMimeData();

    mimeData->setText(fileNames.join("\n"));
    mimeData->setData("RemoteSrc", remoteID_.toUtf8());
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap(":/image/copy.png"));
    drag->exec(Qt::CopyAction);
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

    QStringList fileNames = ClipBoard::fileNames(mimeData);
    fileTransfer(fileNames, QString(), filePath, Upload);

    model_->refresh();
}

void RemoteDockWidget::connected()
{
    std::string homeDir = sftp->homeDir();
    if(homeDir.empty())
        model_->setDir(sftp->home());
    else
        model_->setDir(sftp->dir(homeDir));

    updateCurrentDir(model_->dirName());
}

void RemoteDockWidget::unconnected()
{
    qDebug() << "sftp is disconnected";
}

void RemoteDockWidget::connectionError(QString const& error)
{
    Utils::warring(error);
    emit closeRequest(this);
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

void RemoteDockWidget::newFolder()
{
    QString path = Utils::getText(tr("Folder Name"));
    if(path.isEmpty())
        return;
    if(model_->mkdir(path.toStdString()))
        model_->refresh();
}

void RemoteDockWidget::newTxtFile()
{
    QString fileName = Utils::getText(tr("File Name"));
    if(fileName.isEmpty())
        return;
    if(model_->mkFile(fileName.toStdString()))
        model_->refresh();
}

void RemoteDockWidget::upload()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    QApplication::applicationName());
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
        QString fileName = download(fileInfo, Utils::tempDir());
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

    QString fileName = download(fileInfo, Utils::tempDir());
    if(!fileName.isEmpty())
        WinShell::OpenWith(fileName);
}

void RemoteDockWidget::download()
{
    QStringList fileNames = selectedileNames();
    if(fileNames.isEmpty())
        return;

    QString filePath = QFileDialog::getExistingDirectory(this,
                                                         QApplication::applicationName());
    if(filePath.isEmpty())
        return;

    fileTransfer(fileNames, model_->dirName(), filePath, Download);
}

void RemoteDockWidget::downloadFiles(QString const& remoteSrc,
                                     QStringList const& fileNames,
                                     QString const& targetFilePath)
{
    if(remoteSrc == remoteID_)
        fileTransfer(fileNames, model_->dirName(), targetFilePath, Download);
}

void RemoteDockWidget::delFiles()
{
    fileTransfer(selectedileNames(), model_->dirName(), QString(), Delete);
    model_->refresh();
}

QStringList RemoteDockWidget::selectedileNames()
{
    QModelIndexList indexs = ui->treeView->selectionModel()->selectedRows(0);
    QStringList names;
    for(int i = 0; i < indexs.size(); i++)
        names << QString::fromStdString(model_->fileName(indexs[i].row()));
    return names;
}

void RemoteDockWidget::rename()
{
    QModelIndex index = ui->treeView->currentIndex();
    ssh::FileInfoPtr fileInfo = model_->fileInfo(index.row());
    if(!fileInfo)
        return;
    ui->treeView->edit(ui->treeView->currentIndex());
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
        QMessageBox::warning(this, QApplication::applicationName(), tr("Permission denied"));
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

QString RemoteDockWidget::download(ssh::FileInfoPtr const& fileInfo,
                                   QDir const& dstDir)
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
    fileTransfer(QStringList() << fileName, QString(), model_->dirName(), Upload);
    return true;
}

void RemoteDockWidget::fileTransfer(QStringList const& srcFileNames,
                                    QString const& srcFilePath,
                                    QString const& dstFilePath,
                                    OperateType type)
{
    RemoteFileTransfer transfer(new SFtpFileManager(sftp));
    FileProgressDialog dialog(this);

    connect(&transfer, &RemoteFileTransfer::totalProgress, &dialog, &FileProgressDialog::totalProgress);
    connect(&transfer, &RemoteFileTransfer::fileProgress, &dialog, &FileProgressDialog::fileProgress);
    connect(&transfer, &RemoteFileTransfer::finished, &dialog, &FileProgressDialog::finished);
    connect(&transfer, &RemoteFileTransfer::error, &dialog, &FileProgressDialog::error);
    dialog.setModal(true);
    dialog.show();
    if(type == Download)
    {
        dialog.setWindowTitle(tr("DownloadFiles"));
        transfer.downloadFiles(srcFileNames, srcFilePath, dstFilePath);
    }
    else if(type == Upload)
    {
        dialog.setWindowTitle(tr("UploadFiles"));
        transfer.uploadFiles(srcFileNames, dstFilePath);
    }
    else if(type == Delete)
    {
        dialog.setWindowTitle(tr("DeleteFiles"));
        transfer.deleteFiles(srcFileNames, srcFilePath, false);
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

QString RemoteDockWidget::getStatusText()
{
    QModelIndexList indexs = ui->treeView->selectionModel()->selectedRows(0);
    QStringList names;
    int files = 0;
    int dirs = 0;
    qint64 fileSize = 0;
    for(int i = 0; i < indexs.size(); i++)
    {
        if(model_->isParent(indexs[i].row()))
            continue;

        if(model_->isDir(indexs[i].row()))
            dirs++;
        else
        {
            files++;
            fileSize += model_->fileSize(indexs[i].row());
        }
    }
    return  QString("%1/%2,%3/%4 files,%5/%6 dirs(s)")
            .arg(Utils::formatFileSizeKB(fileSize),
                 Utils::formatFileSizeKB(model_->fileSizes()))
            .arg(files).arg(model_->fileCount())
            .arg(dirs).arg(model_->dirCount());
}

void RemoteDockWidget::updateCurrentDir(QString const& dir,
                                        QString const& caption,
                                        bool  isNavigation)
{
    if(caption.isEmpty())
        titleBarWidget->setTitle(dir);
    else
        titleBarWidget->setTitle(caption);
    if(!isNavigation)
        dirHistory->add(dir);
    emit dirChanged(dir);
    emit statusTextChanged(getStatusText());
}

void RemoteDockWidget::applyOptions()
{
    {
        LayoutOption const& o = theOptionManager.layoutOption();
        RemoteDockWidget::showHeader(o.isShowSortHeader);
        RemoteDockWidget::showCurrentDir(o.isShowCurrentDir);
        RemoteDockWidget::showDeskNavigationButton(o.isShowDeskNavigationButton);
        RemoteDockWidget::showFavoriteButton(o.isShowFavoriteButton);
        RemoteDockWidget::showHistoryButton(o.isShowHistoryButton);
    }
    {
        DisplayOption const& o = theOptionManager.displayOption();
        RemoteDockWidget::showHiddenAndSystem(o.isShowHideAndSystemFile);
        RemoteDockWidget::showParentInRoot(o.isShowParentDirInRootDrive);
        RemoteDockWidget::showToolTips(o.isShowFilenameTooltips);
        RemoteDockWidget::setDirSoryByTime(!o.isDirSortByName);
    }
    {
        IconsOption const& o = theOptionManager.iconOption();
        RemoteDockWidget::showAllIconWithExeAndLink(o.isShowAllIconIncludeExeAndLink);
        RemoteDockWidget::showAllIcon(o.isShowAllIcon);
        RemoteDockWidget::showStandardIcon(o.isShowStandardIcon);
        RemoteDockWidget::showNoneIcon(o.isNoShowIcon);
        RemoteDockWidget::showIconForFyleSystem(o.isShowIconForFilesystem);
        RemoteDockWidget::showIconForVirtualFolder(o.isShowOverlayIcon);
        RemoteDockWidget::fileIconSize(o.fileIconSize);
    }
    {
        FontOption const& o = theOptionManager.fontOption();
        RemoteDockWidget::fileFont(o.fileList.font());
    }
    {
        ColorOption const& o = theOptionManager.colorOption();
        RemoteDockWidget::setItemColor(o.fontColor, o.background1Color, o.background2Color);
        RemoteDockWidget::setItemSelectedColor(o.background1Color, o.markColor, o.cursorColor);
    }
    {
        OperationOption const& o = theOptionManager.operationOption();
        RemoteDockWidget::setRenameFileName(o.isSelectFileNameWhenRenaming);
    }
}
