#include "localdirdockwidget.h"
#include "ui_localdirdockwidget.h"
#include "titlebarwidget.h"
#include "model/localdirmodel.h"
#include "core/filemanager.h"
#include "core/filetransfer.h"
#include "core/clipboard.h"
#include "core/contextmenu.h"
#include "util/utils.h"
#include "dialog/fileprogressdialog.h"
#include "dialog/fileoperateconfirmdialog.h"

#include <QMenu>
#include <QSettings>
#include <QDrag>
#include <QMimeData>

LocalDirDockWidget::LocalDirDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::LocalDirDockWidget)
    , model_(new LocalDirModel(this))
    , titleBarWidget(new TitleBarWidget())
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
}

LocalDirDockWidget::~LocalDirDockWidget()
{
    delete ui;
}

void LocalDirDockWidget::setDir(QString const& dir, QString const& caption)
{
    model_->setDir(dir);
    updateCurrentDir(dir, caption);
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
    if(model_->cd(dir))
    {
        QString dirName = model_->dir();
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
             delFiles();
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
    QString fileName = model_->fileName(index.row());
    if(model_->cd(fileName))
        updateCurrentDir(model_->dir());
    else
        FileManager::Open(model_->filePath(index.row()));
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
            FileManager::OpenByExplorer(fileName);
        });

        QFont font = action->font();
        font.setBold(true);
        action->setFont(font);

        items = ContextMenu::DirCommands();
    }
    else
    {
        fileName = selectedFileName();
        fileName = model_->filePath(index.row());
        fileInfo.setFile(fileName);

        if(fileInfo.isFile())
        {
            QAction* action = menu.addAction(tr("Open with"), this, [=](){
                FileManager::OpenWith(fileInfo.filePath());
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
            QStringList fileNames = selectedileNames(QString(), false, true);
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
        menu.addAction("Delete", this, SLOT(delFiles()));
        menu.addAction("Rename", this, SLOT(rename()));
    }
    else
    {
        menu.addAction("New folder", this, SLOT(newFolder()));
        menu.addAction("New txt file", this, SLOT(newTxtFile()));
    }
    menu.addSeparator();
    menu.addAction("Properties", this, [&](bool) {
        QStringList fileNames = selectedileNames(QString(), false, true);
        if(fileNames.size() > 1)
            FileManager::Property(fileNames);
        else
            FileManager::Property(fileName);
    });

    menu.exec(QCursor::pos());
}

void LocalDirDockWidget::beginDragFile(QPoint const& point)
{
    QModelIndex index = ui->treeView->indexAt(point);
    if(!index.isValid())
        return;
    QDrag *drag = new QDrag(ui->treeView);
    QStringList fileNames = selectedileNames("file:///");
    QMimeData* mimeData = ClipBoard::copyMimeData(fileNames);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap(":/image/copy.png"));
    drag->exec();
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

    QStringList fileNames = mimeData->text().split("\n");
    FileNames  newFileNames = getFileNames(fileNames, filePath);
    fileTransfer(newFileNames, false);
    model_->refresh();
}

void LocalDirDockWidget::cut()
{
    QStringList fileNames = selectedileNames("file:///", false, true);
    ClipBoard::cut(fileNames);
}

void LocalDirDockWidget::copy()
{
    QStringList fileNames = selectedileNames("file:///", false, true);
    ClipBoard::copy(fileNames);
}

void LocalDirDockWidget::paste()
{
    uint32_t dropMask = ClipBoard::dropEffect();
    if(ClipBoard::isCut(dropMask))
        cutFiles(ClipBoard::fileNames(), model_->dir());
    else if(ClipBoard::isCopy(dropMask))
        copyFilels(ClipBoard::fileNames(), model_->dir());
    model_->refresh();
}

void LocalDirDockWidget::delFilesWithConfirm()
{
    QStringList fileNames = selectedileNames(QString(), true);
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
    QStringList fileNames = selectedileNames();
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
        FileManager::Open(fileName);
}
void LocalDirDockWidget::editFile()
{
    QString fileName = selectedFileName();
    QFileInfo fileInfo(fileName);
    if(fileInfo.isFile())
        FileManager::Open(fileName);
}

void LocalDirDockWidget::copyFiles(QString const& dstFilePath)
{
    QStringList fileNames = selectedileNames(QString(), true);
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
        copyFilels(selectedileNames("file:///"), dstFilePath);
}

void LocalDirDockWidget::moveFiles(QString const& dstFilePath)
{
    QStringList fileNames = selectedileNames(QString(), true);
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
    {
        cutFiles(selectedileNames("file:///"), dstFilePath);
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

QStringList LocalDirDockWidget::selectedileNames(QString const& prefix, bool isOnlyFilename, bool isParent)
{
    QModelIndexList indexs = ui->treeView->selectionModel()->selectedRows(0);
    QStringList names;
    for(int i = 0; i < indexs.size(); i++)
    {
        if(isOnlyFilename)
            names << prefix + model_->fileName(indexs[i].row());
        else
            names << prefix + model_->filePath(indexs[i].row());
    }
    if(isParent && names.isEmpty())
        names << prefix + model_->dir();
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

FileNames LocalDirDockWidget::getFileNames(QStringList const& fileNames, QString const& filePath)
{
    FileNames  newFileNames;
    QDir dir(filePath);
    for(int i = 0; i < fileNames.size(); i++)
    {
        if(fileNames[i].isEmpty())
            continue;

        QFileInfo fileInfo(fileNames[i].mid(QString("file:///").size()));

        FileName fileName;
        fileName.src = fileInfo.filePath();
        fileName.dst = dir.filePath(fileInfo.fileName());
        if(fileName.src == fileName.dst)
        {
            QString filename ;
            if(fileInfo.isDir())
                filename = QString("%1_copy").arg(fileInfo.completeBaseName());
            else
                filename = QString("%1_copy.%2").arg(fileInfo.completeBaseName(), fileInfo.suffix());
            fileName.dst = dir.filePath(filename);
        }
        newFileNames << fileName;
    }
    return newFileNames;
}
void LocalDirDockWidget::copyFilels(QStringList const& fileNames, QString const& dstFilePath)
{
    FileNames  newFileNames = getFileNames(fileNames, dstFilePath);
    fileTransfer(newFileNames, false);
}

void LocalDirDockWidget::cutFiles(QStringList const& fileNames, QString const& dstFilePath)
{
    FileNames  newFileNames = getFileNames(fileNames, dstFilePath);
    fileTransfer(newFileNames, true);
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

void LocalDirDockWidget::updateCurrentDir(QString const& dir, QString const& caption)
{
    if(caption.isEmpty())
        titleBarWidget->setTitle(dir);
    else
        titleBarWidget->setTitle(caption);
    emit dirChanged(dir, false);
}
