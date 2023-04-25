#include "localdirdockwidget.h"
#include "ui_localdirdockwidget.h"
#include "model/localfilemodel.h"
#include "core/filemanager.h"
#include "core/filetransfer.h"
#include "core/clipboard.h"
#include "core/contextmenu.h"
#include "dialog/fileprogressdialog.h"

#include <QMenu>

LocalDirDockWidget::LocalDirDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::LocalDirDockWidget)
    , model_(new LocalDirModel(this))
{
    ui->setupUi(this);
    model_->setDir("");
    ui->treeView->setRootIsDecorated(false);
    ui->treeView->setModel(model_);
    ui->treeView->setSortingEnabled(true);
    ui->treeView->installEventFilter(this);

    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)),
                    this, SLOT(viewClick(QModelIndex)));
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)),
                    this, SLOT(customContextMenuRequested(QPoint)));
    connect(ui->treeView->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
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
        model_->sort(logicalIndex, false);
    else
        model_->sort(logicalIndex, true);
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
    menu.addAction("Property", this, SLOT(property()));

    menu.exec(QCursor::pos());
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

void LocalDirDockWidget::property()
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

