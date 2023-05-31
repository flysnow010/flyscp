#include "localdirdockwidget.h"
#include "ui_localdirdockwidget.h"
#include "titlebarwidget.h"
#include "model/localdirmodel.h"
#include "core/filemanager.h"
#include "core/filetransfer.h"
#include "core/winshell.h"
#include "core/clipboard.h"
#include "core/contextmenu.h"
#include "core/filecompresser.h"
#include "core/fileuncompresser.h"
#include "util/utils.h"
#include "dialog/fileprogressdialog.h"
#include "dialog/fileoperateconfirmdialog.h"
#include "dialog/compressconfirmdialog.h"
#include "dialog/uncompressconfirmdialog.h"
#include "dialog/serchfiledialog.h"

#include <QMenu>
#include <QSettings>
#include <QDrag>
#include <QMimeData>
#include <QFileSystemWatcher>

LocalDirDockWidget::LocalDirDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::LocalDirDockWidget)
    , model_(new LocalDirModel(this))
    , titleBarWidget(new TitleBarWidget())
    , fileSystemWatcher(new QFileSystemWatcher(this))
{
    ui->setupUi(this);
    ui->treeView->setModel(model_);
    ui->treeView->installEventFilter(this);
    setTitleBarWidget(titleBarWidget);

    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)),
                    this, SLOT(viewClick(QModelIndex)));
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)),
                    this, SLOT(customContextMenuRequested(QPoint)));
    connect(ui->treeView, SIGNAL(prepareDrag(QPoint)),
                    this, SLOT(beginDragFile(QPoint)));
    connect(ui->treeView, SIGNAL(dragEnter(QDragEnterEvent*)),
                    this, SLOT(dragEnter(QDragEnterEvent*)));
    connect(ui->treeView, SIGNAL(dragMove(QDragMoveEvent*)),
                    this, SLOT(dragMove(QDragMoveEvent*)));
    connect(ui->treeView, SIGNAL(drop(QDropEvent*)),
                    this, SLOT(drop(QDropEvent*)));
    connect(ui->treeView->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
    connect(ui->treeView->header(), SIGNAL(sectionResized(int,int,int)),
            this, SIGNAL(sectionResized(int,int,int)));

    connect(titleBarWidget, SIGNAL(libDirButtonClicked()),
                    this, SIGNAL(libDirContextMenuRequested()));
    connect(titleBarWidget, SIGNAL(favoritesDirButtonCLicked()),
                    this, SIGNAL(favoritesDirContextMenuRequested()));
    connect(titleBarWidget, SIGNAL(historyDirButtonClicked()),
                    this, SIGNAL(historyDirContextMenuRequested()));
    connect(titleBarWidget, &TitleBarWidget::dirSelected, this, [&](QString const& dir)
    {
        setDir(dir);
    });
    connect(fileSystemWatcher, SIGNAL(directoryChanged(QString)),
        this, SLOT(directoryChanged(QString)));
}

LocalDirDockWidget::~LocalDirDockWidget()
{
    delete ui;
}

void LocalDirDockWidget::setDir(QString const& dir, QString const& caption, bool isNavigation)
{
    fileSystemWatcher->removePath(model_->dir());
    model_->setDir(dir);
    fileSystemWatcher->addPath(dir);
    updateCurrentDir(dir, caption, isNavigation);
}

QString LocalDirDockWidget::dir() const
{
    return model_->dir();
}

QString LocalDirDockWidget::home() const
{
    return QDir::homePath();
}

QString LocalDirDockWidget::root() const
{
    QString dir = model_->dir();
    QFileInfoList dirvers = QDir::drives();
    foreach(auto const& dirver, dirvers)
    {
        if(dir.startsWith(dirver.path()))
            return dirver.path();
    }
    return QString();
}

void LocalDirDockWidget::setActived(bool isActived)
{
    titleBarWidget->setActived(isActived);
}

bool LocalDirDockWidget::isActived() const
{
    return titleBarWidget->isActived();
}

void LocalDirDockWidget::cd(QString const& dir)
{
    QString oldDir = model_->dir();
    if(model_->cd(dir))
    {
        QString dirName = model_->dir();
        fileSystemWatcher->removePath(oldDir);
        fileSystemWatcher->addPath(dirName);
        updateCurrentDir(dirName);
    }
}

void LocalDirDockWidget::resizeSection(int logicalIndex, int size)
{
    if(ui->treeView->header()->sectionSize(logicalIndex) != size)
        ui->treeView->header()->resizeSection(logicalIndex, size);
}

void LocalDirDockWidget::saveSettings(QString const& name)
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup(name);
    QHeaderView *headerView = ui->treeView->header();
    settings.setValue("DirName", model_->dir());
    settings.beginWriteArray("sectionSizes", headerView->count());
    for(int i = 0; i < headerView->count(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("sectionSize", headerView->sectionSize(i));
    }
    settings.endArray();
    settings.endGroup();
}

void LocalDirDockWidget::loadSettings(QString const& name)
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup(name);
    cd(settings.value("DirName").toString());
    QHeaderView *headerView = ui->treeView->header();
    int size = settings.beginReadArray("sectionSizes");
    for(int i = 0; i < size && i < headerView->count(); i++)
    {
        settings.setArrayIndex(i);
        headerView->resizeSection(i, settings.value("sectionSize").toInt());
    }
    settings.endArray();
    settings.endGroup();
}

bool LocalDirDockWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
         QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
         if(keyEvent->matches(QKeySequence::Copy))
         {
             copy();
             return true;
         }
         else if(keyEvent->matches(QKeySequence::Cut))
         {
            cut();
            return true;
         }
         else if(keyEvent->matches(QKeySequence::Paste))
         {
             paste();
             return true;
         }
         else if(keyEvent->matches(QKeySequence::Delete))
         {
             delFilesWithConfirm();
             return true;
         }
    }
    else if(event->type() == QEvent::FocusIn)
    {
        setActived(true);
        emit actived();
    }
    return QDockWidget::eventFilter(obj, event);
}

void LocalDirDockWidget::viewClick(QModelIndex const& index)
{
    QFileInfo fileInfo = model_->fileInfo(index.row());

    if(!fileInfo.isDir())
         WinShell::Open(model_->filePath(index.row()));
    else
    {
        if(fileInfo.isSymLink())
        {
            QString dir = QFile::symLinkTarget(fileInfo.filePath());
            model_->setDir(dir);
            updateCurrentDir(model_->dir());
        }
        else if(model_->cd(fileInfo.fileName()))
            updateCurrentDir(model_->dir());
    }
}

void LocalDirDockWidget::directoryChanged(const QString &path)
{
    if(path == model_->dir())
        model_->refresh();
}

void LocalDirDockWidget::sortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    if(order == Qt::SortOrder::AscendingOrder)
        model_->sortItems(logicalIndex, false);
    else
        model_->sortItems(logicalIndex, true);
}

void LocalDirDockWidget::customContextMenuRequested(const QPoint & pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);
    QMenu menu;
    ContextMenuItems items;
    QString fileName;
    QFileInfo fileInfo;

    if(!index.isValid())
    {
        fileInfo.setFile(model_->dir());
        fileName = fileInfo.filePath();

        QString name = fileInfo.fileName();
        if(name.isEmpty())
            name = fileName;

        QAction* action = menu.addAction(name, this, [=]()
        {
            WinShell::OpenByExplorer(fileName);
        });

        QFont font = action->font();
        font.setBold(true);
        action->setFont(font);

        items = ContextMenu::DirCommands();
    }
    else
    {
        fileName = model_->filePath(index.row());
        fileInfo.setFile(fileName);

        if(fileInfo.isFile())
        {
            QAction* action = menu.addAction(tr("Open with"), this, [=](){
                WinShell::OpenWith(fileInfo.filePath());
            });

            QFont font = action->font();
            font.setBold(true);
            action->setFont(font);
            items = ContextMenu::FileCommands();
        }
        else
        {
            QString name = fileInfo.fileName();
            QAction* action = menu.addAction(name, this, [=]()
            {
                if(model_->cd(name))
                    updateCurrentDir(model_->dir());
            });
            QFont font = action->font();
            font.setBold(true);
            action->setFont(font);
            items = ContextMenu::DirCommands();
        }
    }

    foreach(auto const& item, items)
    {
        menu.addAction(item.icon, item.name, [=](bool){
            QStringList fileNames = selectedFileNames(false, true);
            item.exec(fileNames);
        });
    }

    menu.addSeparator();
    menu.addAction("Cut", this, SLOT(cut()));
    menu.addAction("Copy", this, SLOT(copy()));
    if(ClipBoard::canPaste())
        menu.addAction("Paste", this, [&](bool){
            uint32_t dropMask = ClipBoard::dropEffect();
            QString filePath;
            if(fileInfo.isDir())
                filePath = fileInfo.filePath();
            else
                filePath = model_->dir();
            if(ClipBoard::isCut(dropMask))
                cutFiles(ClipBoard::fileNames(), filePath);
            else if(ClipBoard::isCopy(dropMask))
                copyFilels(ClipBoard::fileNames(), filePath);
            ClipBoard::clear();
            model_->refresh();
        });
    menu.addSeparator();
    menu.addAction("Create shortcut", this, SLOT(createShortcut()));
    if(index.isValid())
    {
        if(!model_->isParent(index.row()))
        {
            menu.addAction("Delete", this, SLOT(delFiles()));
            menu.addAction("Rename", this, SLOT(rename()));
        }
    }
    else
    {
        menu.addAction("New folder", this, SLOT(newFolder()));
        menu.addAction("New txt file", this, SLOT(newTxtFile()));
    }
    menu.addSeparator();
    menu.addAction("Properties", this, [&](bool) {
        QStringList fileNames = selectedFileNames(false, true);
        if(fileNames.size() > 1)
            WinShell::Property(fileNames);
        else
            WinShell::Property(fileName);
    });

    menu.exec(QCursor::pos());
}

void LocalDirDockWidget::beginDragFile(QPoint const& point)
{
    QModelIndex index = ui->treeView->indexAt(point);
    if(!index.isValid())
        return;
    QDrag *drag = new QDrag(ui->treeView);
    QStringList fileNames = ClipBoard::fileNames(selectedFileNames());
    QMimeData* mimeData = WinShell::dropMimeData(fileNames);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap(":/image/copy.png"));
    drag->exec(Qt::LinkAction | Qt::MoveAction | Qt::CopyAction , Qt::CopyAction);
}

void LocalDirDockWidget::dragEnter(QDragEnterEvent * event)
{
    QMimeData const* mimeData = event->mimeData();
    if(mimeData)
        event->acceptProposedAction();
}

void LocalDirDockWidget::dragMove(QDragMoveEvent * event)
{
    QModelIndex index = ui->treeView->indexAt(event->pos());
    bool isSelf = (event->source() == ui->treeView);
    if(index.isValid())
    {
        if(isSelf && model_->fileInfo(index.row()).isFile())
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

void LocalDirDockWidget::drop(QDropEvent * event)
{
    QModelIndex index = ui->treeView->indexAt(event->pos());
    QString filePath;
    if(!index.isValid())
        filePath = model_->dir();
    else
    {
        if(model_->fileInfo(index.row()).isFile())
            filePath = model_->dir();
        else
            filePath = model_->filePath(index.row());
    }

    QMimeData const* mimeData = event->mimeData();
    if(!mimeData)
        return;

    QStringList fileNames = ClipBoard::fileNames(mimeData);
    if(event->dropAction() == Qt::MoveAction)
    {
        if(!Utils::question(QString("Move %1 files or folders to\n%2").arg(fileNames.size()).arg(filePath)))
            return;
        fileTransfer(FileNames::GetFileNames(fileNames, filePath), true);
    }
    else if(event->dropAction() == Qt::CopyAction)
    {
        if(!Utils::question(QString("Copy %1 files or folders to\n%2").arg(fileNames.size()).arg(filePath)))
            return;
        fileTransfer(FileNames::GetFileNames(fileNames, filePath), false);
    }
    else if(event->dropAction() == Qt::LinkAction)
    {
        if(!Utils::question(QString("Create %1 shortcuts in %2").arg(fileNames.size()).arg(filePath)))
            return;
        foreach(auto const& fileName,  fileNames)
        {
            WinShell::CreateShortcut(QString("%1 - shortcut.lnk").arg(fileName), fileName);
        }
    }
    model_->refresh();
}

void LocalDirDockWidget::cut()
{
    QStringList fileNames = selectedFileNames(false, true);
    WinShell::Copy(fileNames, true);
}

void LocalDirDockWidget::copy()
{
    QStringList fileNames = selectedFileNames(false, true);
    WinShell::Copy(fileNames, false);
}

void LocalDirDockWidget::paste()
{
    uint32_t dropMask = ClipBoard::dropEffect();
    if(ClipBoard::isCut(dropMask))//判断ClipBoard::fileNames()是否为空
        cutFiles(ClipBoard::fileNames(), model_->dir());
    else if(ClipBoard::isCopy(dropMask))
        copyFilels(ClipBoard::fileNames(), model_->dir());
    model_->refresh();
}

void LocalDirDockWidget::delFilesWithConfirm()
{
    QStringList fileNames = selectedFileNames(true);
    QString tipText;
    if(fileNames.size() > 1)
        tipText = QString("Are you sure you want to delete %1 files or folders?\n\n%2").arg(fileNames.size())
                .arg(fileNames.join("\n"));
    else
    {
        QFileInfo fileInfo(selectedFileName());
        QString type;
        if(fileInfo.isDir())
            type = "folder";
        else
            type = "file";
        tipText = QString("Are you sure you want to delete the %1 %2?").arg(type, fileNames.first());
    }

    if(Utils::question(tipText))
        delFiles();
}

void LocalDirDockWidget::delFiles()
{
    QStringList fileNames = selectedFileNames();
    FileManager fileManger;
    fileManger.delereFiles(fileNames);
    model_->refresh();
}

void LocalDirDockWidget::rename()
{
    QString fileName = selectedFileName(true);
    if(fileName.isEmpty())
        return;
    QString newileName = Utils::getText(tr("New filename"), fileName);
    if(newileName.isEmpty() ||  newileName == fileName)
        return;

    if(model_->rename(fileName, newileName))
        model_->refresh();
}

void LocalDirDockWidget::createShortcut()
{
    QStringList fileNames = selectedFileNames(false, true);
    foreach(auto const& fileName,  fileNames)
    {
        WinShell::CreateShortcut(QString("%1 - shortcut.lnk").arg(fileName), fileName);
    }
    model_->refresh();
}

void LocalDirDockWidget::newFolder()
{
    QString path = Utils::getText("New folder");
    if(path.isEmpty())
        return;
    if(model_->mkdir(path))
        model_->refresh();
}

void LocalDirDockWidget::viewFile()
{
    QString fileName = selectedFileName();
    QFileInfo fileInfo(fileName);
    if(fileInfo.isFile())
        WinShell::Exec(Utils::viewApp(), fileName);
}
void LocalDirDockWidget::editFile()
{
    QString fileName = selectedFileName();
    QFileInfo fileInfo(fileName);
    if(fileInfo.isFile())
        WinShell::Exec(Utils::editApp(), fileName);
}

void LocalDirDockWidget::copyFiles(QString const& dstFilePath)
{
    QStringList fileNames = selectedFileNames(true);
    if(fileNames.isEmpty())
    {
        Utils::warring("No files or folders selected!");
        return;
    }

    FileOperateConfirmDialog dialog;
    if(fileNames.size() > 1)
        dialog.setLabel(QString("Copy %1 files or folders to:").arg(fileNames.size()));
    else
    {
        QFileInfo fileInfo(selectedFileName());
        QString type;
        if(fileInfo.isDir())
            type = tr("folder");
        else
            type = tr("file");
        dialog.setLabel(QString("Copy the %1 %2 to:").arg(type, fileNames.first()));
    }
    dialog.setPath(dstFilePath);
    if(dialog.exec() == QDialog::Accepted)
        copyFilels(selectedFileNames(), dstFilePath);
}

void LocalDirDockWidget::moveFiles(QString const& dstFilePath)
{
    QStringList fileNames = selectedFileNames(true);
    if(fileNames.isEmpty())
    {
        Utils::warring("No files or folders selected!");
        return;
    }
    FileOperateConfirmDialog dialog;
    if(fileNames.size() > 1)
        dialog.setLabel(QString("Move %1 files or folders to:").arg(fileNames.size()));
    else
    {
        QFileInfo fileInfo(selectedFileName());
        QString type;
        if(fileInfo.isDir())
            type = tr("folder");
        else
            type = tr("file");
        dialog.setLabel(QString("Move the %1 %2 to:").arg(type, fileNames.first()));
    }
    dialog.setPath(dstFilePath);
    if(dialog.exec() == QDialog::Accepted)
    {
        cutFiles(selectedFileNames(), dstFilePath);
        model_->refresh();
    }
}

void LocalDirDockWidget::refresh()
{
    model_->refresh();
}

void LocalDirDockWidget::selectAll()
{
    ui->treeView->selectAll();
    if(model_->isParent(0))
    {
        QModelIndexList indexList = ui->treeView->selectionModel()->selectedColumns(0);
        foreach(auto const& index, indexList)
        {
            ui->treeView->selectionModel()->select(index, QItemSelectionModel::Deselect);
        }
    }
}

void LocalDirDockWidget::compressFiles(QString const& dstFilePath)
{
    QStringList fileNames = selectedFileNames();
    if(fileNames.isEmpty())
    {
        Utils::warring("No select files!");
        return;
    }

    {
        CompressConfirmDialog d(this);
        QString fileName;
        if(fileNames.size() > 1)
        {
            QString baseName = QFileInfo(model_->dir()).baseName();
            if(baseName.isEmpty())
                baseName = "archive";
            fileName = baseName + CompressConfirmDialog::CurrentSuffix();
        }
        else
        {
            if(QFileInfo(fileNames[0]).isDir())
                fileName = fileNames[0] + CompressConfirmDialog::CurrentSuffix();
            else
                fileName = QFileInfo(fileNames[0]).completeBaseName() + CompressConfirmDialog::CurrentSuffix();
        }

        d.adjustSize();
        d.setFileNames(fileNames);
        d.setTargetFileName(QDir(dstFilePath).filePath(fileName));
        if(d.exec() == QDialog::Accepted)
        {
            QString targetFileName = d.targetFileName();
            CompressParam param = d.settings();
            FileCompresser compresser;
            FileProgressDialog dialog(this);
            dialog.setStatusTextMode();

            connect(&compresser, &FileCompresser::progress, &dialog, &FileProgressDialog::progressText);
            connect(&compresser, &FileCompresser::finished, &dialog, &FileProgressDialog::finished);
            connect(&compresser, &FileCompresser::error, &dialog, &FileProgressDialog::error);

            dialog.setModal(true);
            dialog.show();
            compresser.compress(fileNames, param, targetFileName);
            while(!dialog.isFinished())
            {
                if(dialog.isCancel())
                {
                    compresser.cancel();
                    while(!dialog.isFinished())
                        QApplication::processEvents();
                }
                QApplication::processEvents();
            }
            if(param.isMoveFile)
                model_->refresh();
        }
    }
}

void LocalDirDockWidget::uncompressFiles(QString const& dstFilePath)
{
    QStringList fileNames = selectedFileNames();
    if(fileNames.isEmpty())
    {
        Utils::warring("No select files!");
        return;
    }

    QString fileName;
    if(!FileUncompresser::isCompressFiles(fileNames, fileName))
        Utils::warring(QString("Compression package error!\n%1").arg(fileName));
    else
    {
        UnCompressConfirmDialog d(this);
        d.setTargetPath(dstFilePath);
        if(d.exec() == QDialog::Accepted)
        {
            UncompressParam param = d.settings();
            QString targetPath = d.targetPath();

            FileUncompresser uncompresser;
            FileProgressDialog dialog(this);
            dialog.setStatusTextMode();
            if(uncompresser.isEncrypted(fileNames[0]))
                param.password = Utils::getPassword("Input password");

            connect(&uncompresser, &FileUncompresser::progress, &dialog, &FileProgressDialog::progressText);
            connect(&uncompresser, &FileUncompresser::finished, &dialog, &FileProgressDialog::finished);
            connect(&uncompresser, &FileUncompresser::error, &dialog, &FileProgressDialog::error);

            dialog.setModal(true);
            dialog.show();

            uncompresser.uncompress(fileNames, param, targetPath);
            while(!dialog.isFinished())
            {
                if(dialog.isCancel())
                {
                    uncompresser.cancel();
                    while(!dialog.isFinished())
                        QApplication::processEvents();
                }
                QApplication::processEvents();
            }
        }
    }
}

void LocalDirDockWidget::searchFiles(QString const& dstFilePath)
{
    SerchFileDialog dialog;
    dialog.setSearchPath(dstFilePath);
    dialog.exec();
}

void LocalDirDockWidget::newTxtFile()
{
    QString fileName = Utils::getText("New File", "*.txt");
    if(fileName.isEmpty())
        return;
    QFile file(model_->filePath(fileName));
    if(file.open(QIODevice::WriteOnly))
    {
        file.close();
        model_->refresh();
    }
}

bool LocalDirDockWidget::isMultiSelected()
{
    return ui->treeView->selectionModel()->selectedRows(0).size() > 1;
}

bool LocalDirDockWidget::isCompressFiles(QStringList const& fileNames)
{
    QStringList suffixs = QStringList() << "zip" << "7z" << "wim" << "tar"
                                        << "gz" << "xz" << "bz2" << "iso";
    foreach(auto const& fileName, fileNames)
    {
        if(!suffixs.contains(QFileInfo(fileName).suffix().toLower()))
            return false;
    }
    return true;
}

QStringList LocalDirDockWidget::selectedFileNames(bool isOnlyFilename, bool isParent)
{
    QModelIndexList indexs = ui->treeView->selectionModel()->selectedRows(0);
    QStringList names;
    for(int i = 0; i < indexs.size(); i++)
    {
        if(model_->isParent(indexs[i].row()))
            continue;

        if(isOnlyFilename)
            names << model_->fileName(indexs[i].row());
        else
            names << model_->filePath(indexs[i].row());
    }
    if(isParent && names.isEmpty())
        names << model_->dir();
    return names;
}

QString LocalDirDockWidget::selectedFileName(bool isOnlyFilename)
{
    QModelIndex index = ui->treeView->currentIndex();
    if(!index.isValid())
        return QString();
    if(isOnlyFilename)
        return model_->fileName(index.row());
    return model_->filePath(index.row());
}

void LocalDirDockWidget::copyFilels(QStringList const& fileNames, QString const& dstFilePath)
{
    fileTransfer(FileNames::GetFileNames(fileNames, dstFilePath), false);
}

void LocalDirDockWidget::cutFiles(QStringList const& fileNames, QString const& dstFilePath)
{
    fileTransfer(FileNames::GetFileNames(fileNames, dstFilePath), true);
}

void LocalDirDockWidget::fileTransfer(FileNames const& fileNames, bool isMove)
{
    FileTransfer transfer;
    FileProgressDialog dialog(this);

    connect(&transfer, &FileTransfer::totalProgress, &dialog, &FileProgressDialog::totalProgress);
    connect(&transfer, &FileTransfer::fileProgress, &dialog, &FileProgressDialog::fileProgress);
    connect(&transfer, &FileTransfer::finished, &dialog, &FileProgressDialog::finished);
    connect(&transfer, &FileTransfer::error, &dialog, &FileProgressDialog::error);

    dialog.setModal(true);
    dialog.show();
    if(isMove)
    {
        dialog.setWindowTitle("MoveFiles");
        transfer.moveFiles(fileNames);
    }
    else
    {
        dialog.setWindowTitle("CopyFiles");
        transfer.copyFiles(fileNames);
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

void LocalDirDockWidget::updateCurrentDir(QString const& dir, QString const& caption, bool isNavigation)
{
    if(caption.isEmpty())
        titleBarWidget->setTitle(dir);
    else
        titleBarWidget->setTitle(caption);
    emit dirChanged(dir, isNavigation);
}
