#include "localdirdockwidget.h"
#include "ui_localdirdockwidget.h"
#include "model/localdirmodel.h"
#include "core/filemanager.h"
#include "core/filetransfer.h"
#include "core/clipboard.h"
#include "core/contextmenu.h"
#include "util/utils.h"
#include "dialog/fileprogressdialog.h"

#include <QMenu>
#include <QSettings>
#include <QDrag>
#include <QMimeData>


LocalDirDockWidget::LocalDirDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::LocalDirDockWidget)
    , model_(new LocalDirModel(this))
{
    ui->setupUi(this);
    ui->treeView->setModel(model_);
    ui->treeView->installEventFilter(this);

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
}

LocalDirDockWidget::~LocalDirDockWidget()
{
    delete ui;
}

void LocalDirDockWidget::setDir(QString const& dir)
{
    model_->setDir(dir);
    setWindowTitle(dir);
}

QString LocalDirDockWidget::dir()
{
    return model_->dir();
}

void LocalDirDockWidget::cd(QString const& dir)
{
    if(model_->cd(dir))
        setWindowTitle(model_->dir());
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
        settings.value("sectionSize").toInt();
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
             delfile();
             return true;
         }
    }
    return QDockWidget::eventFilter(obj, event);
}

void LocalDirDockWidget::viewClick(QModelIndex const& index)
{
    QString fileName = model_->fileName(index.row());
    if(model_->cd(fileName))
        setWindowTitle(model_->dir());
    else
    {
        QString filePath = model_->filePath(index.row());
        FileManager::Execute(filePath);
    }
}

void LocalDirDockWidget::sortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    if(order == Qt::SortOrder::AscendingOrder)
        model_->sortItems(logicalIndex, false);
    else
        model_->sortItems(logicalIndex, true);
}

void LocalDirDockWidget::customContextMenuRequested(const QPoint &)
{
    QString fileName = selectFileName();
    QFileInfo fileInfo(fileName);
    ContextMenuItems items;
    if(!fileInfo.isDir())
        items = ContextMenu::FileCommands();
    else
        items = ContextMenu::DirCommands();

    QMenu menu;
    foreach(auto const& item, items)
    {
        menu.addAction(item.icon, item.name, [=](bool){
            item.exec(fileName);
        });
    }

    menu.addSeparator();
    menu.addAction(QIcon(":/image/cut.png"), "Cut", this, SLOT(cut()));
    menu.addAction(QIcon(":/image/copy.png"), "Copy", this, SLOT(copy()));
    if(ClipBoard::canPaste())
        menu.addAction(QIcon(":/image/paste.png"), "Paste", this, SLOT(paste()));
    menu.addSeparator();
    menu.addAction("Delete", this, SLOT(delfile()));
    menu.addAction("Rename", this, SLOT(rename()));
    menu.addSeparator();
    menu.addAction("Properties", this, SLOT(properties()));

    menu.exec(QCursor::pos());
}

void LocalDirDockWidget::beginDragFile(QPoint const& point)
{
    QModelIndex index = ui->treeView->indexAt(point);
    if(!index.isValid())
        return;
    QDrag *drag = new QDrag(ui->treeView);
    QMimeData* mimeData = new QMimeData();
    QString fileName = selectFileName();
    mimeData->setText(fileName);
    drag->setMimeData(mimeData);

    drag->setPixmap(Utils::fileIcon("").pixmap(32, 32));
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
}

void LocalDirDockWidget::cut()
{
    QStringList fileNames = selectedileNames("file:///");
    ClipBoard::cut(fileNames);
}

void LocalDirDockWidget::copy()
{
    QStringList fileNames = selectedileNames("file:///");
    ClipBoard::copy(fileNames);
}

void LocalDirDockWidget::paste()
{
    uint32_t dropMask = ClipBoard::dropEffect();
    if(ClipBoard::isCut(dropMask))
        cutFiles(ClipBoard::fileNames());
    else if(ClipBoard::isCopy(dropMask))
        copyFilels(ClipBoard::fileNames());
    model_->update();
}

void LocalDirDockWidget::delfile()
{
    QStringList fileNames = selectedileNames();
    FileManager fileManger;
    fileManger.delereFiles(fileNames);
    model_->update();
}

void LocalDirDockWidget::rename()
{
    QFileInfo fileInfo(selectFileName());
    ContextMenuItems items;
    if(!fileInfo.isDir())
        items = ContextMenu::FileCommands();
    else
        items = ContextMenu::DirCommands();
    ContextMenu::PrintCommand(items);
}

void LocalDirDockWidget::properties()
{
    QModelIndex index = ui->treeView->currentIndex();
    if(!index.isValid())
        return;

    FileManager::Property(model_->filePath(index.row()));
}

QStringList LocalDirDockWidget::selectedileNames(QString const& prefix)
{
    QModelIndexList indexs = ui->treeView->selectionModel()->selectedRows(0);
    QStringList names;
    for(int i = 0; i < indexs.size(); i++)
        names << prefix + model_->filePath(indexs[i].row());
    return names;
}

QString LocalDirDockWidget::selectFileName()
{
    QModelIndex index = ui->treeView->currentIndex();
    if(!index.isValid())
        return QString();
    return model_->filePath(index.row());
}

void LocalDirDockWidget::copyFilels(QStringList const& fileNames)
{
    FileNames  newFileNames;
    for(int i = 0; i < fileNames.size(); i++)
    {
        if(fileNames[i].isEmpty())
            continue;

        QFileInfo fileInfo(fileNames[i].mid(QString("file:///").size()));

        FileName fileName;
        fileName.src = fileInfo.filePath();
        fileName.dst = model_->filePath(fileInfo.fileName());
        newFileNames << fileName;
    }
    fileTransfer(newFileNames, false);
}

void LocalDirDockWidget::cutFiles(QStringList const& fileNames)
{
    FileNames  newFileNames;
    for(int i = 0; i < fileNames.size(); i++)
    {
        if(fileNames[i].isEmpty())
            continue;

        QFileInfo fileInfo(fileNames[i].mid(QString("file:///").size()));
        FileName fileName;
        fileName.src = fileInfo.filePath();
        fileName.dst = model_->filePath(fileInfo.fileName());
        newFileNames << fileName;
    }
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

