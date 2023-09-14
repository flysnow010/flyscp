#include "localdirdockwidget.h"
#include "ui_localdirdockwidget.h"
#include "titlebarwidget.h"
#include "model/localdirmodel.h"
#include "model/compressdirmodel.h"
#include "core/filemanager.h"
#include "core/filetransfer.h"
#include "core/winshell.h"
#include "core/dirhistory.h"
#include "core/dirfavorite.h"
#include "core/clipboard.h"
#include "core/contextmenu.h"
#include "core/filecompresser.h"
#include "core/fileuncompresser.h"
#include "core/compressfileinfo.h"
#include "util/utils.h"
#include "dialog/fileprogressdialog.h"
#include "dialog/fileoperateconfirmdialog.h"
#include "dialog/compressconfirmdialog.h"
#include "dialog/uncompressconfirmdialog.h"
#include "dialog/propertydialog.h"
#include "dialog/searchfiledialog.h"
#include "dialog/favoritesettingsdialog.h"

#include <QMenu>
#include <QSettings>
#include <QDrag>
#include <QMimeData>
#include <QFileSystemWatcher>

LocalDirDockWidget::LocalDirDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::LocalDirDockWidget)
    , model_(new LocalDirModel(this))
    , compressModel_(new CompressDirModel(this))
    , titleBarWidget(new TitleBarWidget())
    , hideBarWidget(new QWidget(this))
    , dirFavorite(new DirFavorite())
    , dirHistory(new DirHistory())
    , fileSystemWatcher(new QFileSystemWatcher(this))
{
    ui->setupUi(this);
    ui->tvNormal->setModel(model_);
    ui->tvCompress->setModel(compressModel_);
    ui->tvCompress->hide();
    ui->tvNormal->installEventFilter(this);
    ui->tvCompress->installEventFilter(this);
    titleBarWidget->installEventFilter(this);
    setTitleBarWidget(titleBarWidget);

    connect(ui->tvNormal, SIGNAL(doubleClicked(QModelIndex)),
                    this, SLOT(normalDoubleClick(QModelIndex)));
    connect(ui->tvCompress, SIGNAL(doubleClicked(QModelIndex)),
                    this, SLOT(compressDoubleClick(QModelIndex)));
    connect(ui->tvNormal, SIGNAL(customContextMenuRequested(QPoint)),
                    this, SLOT(customNormalContextMenu(QPoint)));
    connect(ui->tvCompress, SIGNAL(customContextMenuRequested(QPoint)),
                    this, SLOT(customCompressContextMenu(QPoint)));
    connect(ui->tvNormal, SIGNAL(prepareDrag(QPoint)),
                    this, SLOT(normalBeginDragFile(QPoint)));
    connect(ui->tvNormal, SIGNAL(dragEnter(QDragEnterEvent*)),
                    this, SLOT(normalDragEnter(QDragEnterEvent*)));
    connect(ui->tvNormal, SIGNAL(dragMove(QDragMoveEvent*)),
                    this, SLOT(normalDragMove(QDragMoveEvent*)));
    connect(ui->tvNormal, SIGNAL(drop(QDropEvent*)),
                    this, SLOT(normalDrop(QDropEvent*)));
    connect(ui->tvCompress, SIGNAL(prepareDrag(QPoint)),
                    this, SLOT(compressBeginDragFile(QPoint)));
    connect(ui->tvCompress, SIGNAL(dragEnter(QDragEnterEvent*)),
                    this, SLOT(compressDragEnter(QDragEnterEvent*)));
    connect(ui->tvCompress, SIGNAL(dragMove(QDragMoveEvent*)),
                    this, SLOT(compressDragMove(QDragMoveEvent*)));
    connect(ui->tvCompress, SIGNAL(drop(QDropEvent*)),
                    this, SLOT(compressDrop(QDropEvent*)));
    connect(ui->tvNormal->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
    connect(ui->tvCompress->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
    connect(ui->tvNormal->header(), SIGNAL(sectionResized(int,int,int)),
            this, SIGNAL(sectionResized(int,int,int)));

    connect(ui->tvNormal->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [=]()
    {
            emit statusTextChanged(getStatusText());
    });

    connect(ui->tvCompress->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [=]()
    {
            emit statusTextChanged(getStatusText());
    });

    connect(titleBarWidget, SIGNAL(libDirButtonClicked()),
                    this, SIGNAL(libDirContextMenuRequested()));
    connect(titleBarWidget, SIGNAL(favoritesDirButtonCLicked()),
                    this, SLOT(favoritesDirContextMenu()));
    connect(titleBarWidget, SIGNAL(historyDirButtonClicked()),
                    this, SLOT(historyDirContextMenu()));
    connect(titleBarWidget, &TitleBarWidget::actived, this, [=](){
        emit actived(dir());
        setActived(true);
    });
    connect(titleBarWidget, &TitleBarWidget::dirSelected,
            this, [&](QString const& dir) { setDir(Utils::toLinuxPath(dir)); });
    connect(fileSystemWatcher, SIGNAL(directoryChanged(QString)),
        this, SLOT(directoryChanged(QString)));
}

LocalDirDockWidget::~LocalDirDockWidget()
{
    delete titleBarWidget;
    delete dirFavorite;
    delete dirHistory;
    delete ui;
}

void LocalDirDockWidget::setDir(QString const& dir,
                                QString const& caption,
                                bool isNavigation)
{
    QDir dirInfo(dir);
    if(showMode() == Normal)
    {
        if(!dirInfo.exists())
        {
            setShowMode(Compress);
            compressModel_->setDir(dir);
        }
        else
        {
            fileSystemWatcher->removePath(model_->dir());
            model_->setDir(dir);
            fileSystemWatcher->addPath(dir);
        }
        updateCurrentDir(dir, caption, isNavigation);

    }
    else if(showMode() == Compress)
    {
        if(!dirInfo.exists())
            compressModel_->setDir(dir);
        else
        {
            setShowMode(Normal);
            fileSystemWatcher->removePath(model_->dir());
            model_->setDir(dir);
            fileSystemWatcher->addPath(dir);
        }
        updateCurrentDir(dir, caption, isNavigation);
    }
    if(!isActived())
    {
        emit actived(this->dir());
        setActived(true);
    }
}

QString LocalDirDockWidget::dir() const
{
    return model_->dir();
}

QStringList LocalDirDockWidget::selectedFileNames() const
{
    return getSelectedFileNames();
}

QString LocalDirDockWidget::findDir(QString const& prefix) const
{
    return dirHistory->find(prefix);
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

void LocalDirDockWidget::showHeader(bool isShow)
{
    ui->tvNormal->setHeaderHidden(!isShow);
}

void LocalDirDockWidget::showCurrentDir(bool isShow)
{
    titleBarWidget->setVisible(isShow);
    if(!isShow)
        setTitleBarWidget(hideBarWidget);
    else
        setTitleBarWidget(titleBarWidget);
}

void LocalDirDockWidget::showDeskNavigationButton(bool isShow)
{
    titleBarWidget->showLibDirButton(isShow);
}

void LocalDirDockWidget::showFavoriteButton(bool isShow)
{
    titleBarWidget->showFavoriteButton(isShow);
}

void LocalDirDockWidget::showHistoryButton(bool isShow)
{
    titleBarWidget->showHistoryButton(isShow);
}

void LocalDirDockWidget::showHiddenAndSystem(bool isShow)
{
    model_->showHidden(isShow);
    model_->showSystem(isShow);
}

void LocalDirDockWidget::showToolTips(bool isShow)
{
    model_->showToolTips(isShow);
    compressModel_->showToolTips(isShow);
}

void LocalDirDockWidget::showParentInRoot(bool isShow)
{
    model_->showParentInRoot(isShow);
    compressModel_->showParentInRoot(isShow);
}

void LocalDirDockWidget::setDirSoryByTime(bool isOn)
{
    model_->setDirSortByTime(isOn);
    compressModel_->setDirSortByTime(isOn);
}

void LocalDirDockWidget::setRenameFileName(bool isOn)
{
    model_->setRenameBaseName(isOn);
    compressModel_->setRenameBaseName(isOn);
}

void LocalDirDockWidget::showAllIconWithExeAndLink(bool isShow)
{
    if(isShow)
        model_->setIconShowType(DirModel::ALLWithExeAndLink);
}

void LocalDirDockWidget::showAllIcon(bool isShow)
{
    if(isShow)
        model_->setIconShowType(DirModel::All);
}

void LocalDirDockWidget::showStandardIcon(bool isShow)
{
    if(isShow)
        model_->setIconShowType(DirModel::Standard);
}

void LocalDirDockWidget::showNoneIcon(bool isShow)
{
    if(isShow)
        model_->setIconShowType(DirModel::None);
}

void LocalDirDockWidget::showIconForFyleSystem(bool isShow)
{
    model_->showIconForFyleSystem(isShow);
}

void LocalDirDockWidget::showIconForVirtualFolder(bool isShow)
{
    model_->showIconForVirtualFolder(isShow);
}

void LocalDirDockWidget::showOverlayIcon(bool isShow)
{
    model_->showOverlayIcon(isShow);
}

void LocalDirDockWidget::fileIconSize(int size)
{
    ui->tvNormal->setIconSize(QSize(size, size));
    ui->tvCompress->setIconSize(QSize(size, size));
}

void LocalDirDockWidget::fileFont(QFont const& font)
{
    ui->tvNormal->setFont(font);
    ui->tvCompress->setFont(font);
    if(ui->tvNormal->header())
        ui->tvNormal->header()->setFont(font);
    if(ui->tvCompress->header())
        ui->tvCompress->header()->setFont(font);
}

void LocalDirDockWidget::setItemColor(QString const& fore,                 
                                      QString const& back,
                                      QString const&alternate)
{
    model_->setTextColor(fore);
    model_->setBackground(back);
    model_->setAltColor(alternate);

    compressModel_->setTextColor(fore);
    compressModel_->setBackground(back);
    compressModel_->setAltColor(alternate);

}

void LocalDirDockWidget::setItemSelectedColor(QString const& back,
                  QString const& mark,
                  QString const&cursor)
{
    QString styleSheet = QString("QTreeView{ background: %1;}"
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
                         .arg(back, cursor, mark, model_->textColor());
    ui->tvNormal->setStyleSheet(styleSheet);
    ui->tvCompress->setStyleSheet(styleSheet);
}

void LocalDirDockWidget::setActived(bool isActived)
{
    titleBarWidget->setActived(isActived);
}

bool LocalDirDockWidget::isActived() const
{
    return titleBarWidget->isActived();
}

void LocalDirDockWidget::retranslateUi()
{
    ui->retranslateUi(this);
    model_->reset();
    compressModel_->reset();
    emit statusTextChanged(getStatusText());
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

void LocalDirDockWidget::preDir()
{
    QString newDir = dirHistory->pre(dir());
    if(!newDir.isEmpty())
        setDir(newDir, QString(), true);
}

void LocalDirDockWidget::nextDir()
{
    QString newDir = dirHistory->next(dir());
    if(!newDir.isEmpty())
        setDir(newDir, QString(), true);
}

void LocalDirDockWidget::resizeSection(int logicalIndex,
                                       int size)
{
    if(ui->tvNormal->header()->sectionSize(logicalIndex) != size)
        ui->tvNormal->header()->resizeSection(logicalIndex, size);
}

void LocalDirDockWidget::saveSettings(QString const& name)
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());

    settings.beginGroup(name);
    settings.setValue("DirName", model_->dir());

    QHeaderView *headerView = ui->tvNormal->header();
    settings.beginWriteArray("sectionSizes", headerView->count());
    for(int i = 0; i < headerView->count(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("sectionSize", headerView->sectionSize(i));
    }
    settings.endArray();

    QStringList const& dirNames = dirHistory->dirs();

    settings.beginWriteArray("historyDirNames", dirNames.size());
    for(int i = 0; i < dirNames.size(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("dirName", dirNames[i]);
    }
    settings.endArray();

    QList<FavoriteItem> items = dirFavorite->favoriteItems();

    settings.beginWriteArray("favoriteItems", items.size());
    for(int i = 0; i < items.size(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("caption", items[i].caption);
        settings.setValue("command", items[i].command);
        settings.setValue("fileName", items[i].fileName);
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

    QHeaderView *headerView = ui->tvNormal->header();
    QHeaderView *compresSheaderView = ui->tvCompress->header();
    int size = settings.beginReadArray("sectionSizes");
    for(int i = 0; i < size && i < headerView->count(); i++)
    {
        settings.setArrayIndex(i);
        headerView->resizeSection(i, settings.value("sectionSize").toInt());
        compresSheaderView->resizeSection(i, settings.value("sectionSize").toInt());
    }
    settings.endArray();

    QStringList dirNames;
    size = settings.beginReadArray("historyDirNames");
    for(int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        QString dirName = settings.value("dirName").toString();
        if(QDir(dirName).exists())
            dirNames << dirName;
    }
    dirHistory->setDirs(dirNames);
    settings.endArray();

    size = settings.beginReadArray("favoriteItems");
    QList<FavoriteItem> items;
    for(int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        FavoriteItem item;
        item.caption = settings.value("caption").toString();
        item.command = settings.value("command").toString();
        item.fileName = settings.value("fileName").toString();
        items << item;
    }
    dirFavorite->setFavoriteItems(items);
    settings.endArray();

    settings.endGroup();
}

bool LocalDirDockWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
         QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
         if(keyEvent->matches(QKeySequence::Copy) && obj == ui->tvNormal)
         {
             copy();
             return true;
         }
         else if(keyEvent->matches(QKeySequence::Cut) && obj == ui->tvNormal)
         {
            cut();
            return true;
         }
         else if(keyEvent->matches(QKeySequence::Paste) && obj == ui->tvNormal)
         {
             paste();
             return true;
         }
         else if(keyEvent->matches(QKeySequence::Delete) && obj == ui->tvNormal)
         {
             deleteFiles(true);
             return true;
         }
    }
    else if(event->type() == QEvent::FocusIn)
    {
        emit actived(dir());
        setActived(true);
    }
    return QDockWidget::eventFilter(obj, event);
}

void LocalDirDockWidget::normalDoubleClick(QModelIndex const& index)
{
    QFileInfo fileInfo = model_->fileInfo(index.row());

    if(!fileInfo.isDir())
    {
        if(!isCompressFiles(fileInfo.suffix()))
            WinShell::Open(model_->filePath(index.row()));
        else
        {
            compressModel_->setCompressFileName(fileInfo.filePath());
            setShowMode(Compress);
            updateCurrentDir(compressModel_->dir());
        }
    }
    else
    {
        if(!fileInfo.isSymLink())
            cd(fileInfo.isRoot() ? fileInfo.filePath(): fileInfo.fileName());
        else
        {
            QString dir = QFile::symLinkTarget(fileInfo.filePath());
            model_->setDir(dir);
            updateCurrentDir(model_->dir());
        }
    }
}

void LocalDirDockWidget::compressDoubleClick(QModelIndex const& index)
{
    CompressFileInfo::Ptr fileInfo = compressModel_->fileInfo(index.row());
    if(fileInfo->isDir())
    {
        if(compressModel_->cd(fileInfo->path()))
            updateCurrentDir(compressModel_->dir());
        else
        {
            setShowMode(Normal);
            updateCurrentDir(model_->dir());
        }
    }
    else
    {
        QDir targetDir = Utils::tempDir();
        if(compressModel_->extract(targetDir.path(), QStringList() << fileInfo->filePath(), false))
        {
            QString fileName = targetDir.filePath(fileInfo->fileName());
            FileNames::MakeFileNameAsParams(fileName);
            WinShell::Open(fileName);
        }
    }
}

void LocalDirDockWidget::directoryChanged(const QString &path)
{
    if(path == model_->dir())
        model_->refresh();
}

void LocalDirDockWidget::sortIndicatorChanged(int logicalIndex,
                                              Qt::SortOrder order)
{
    if(sender() == ui->tvNormal->header())
    {
        if(order == Qt::SortOrder::AscendingOrder)
            model_->sortItems(logicalIndex, false);
        else
            model_->sortItems(logicalIndex, true);
    }
    else if(sender() == ui->tvCompress->header())
    {
        if(order == Qt::SortOrder::AscendingOrder)
            compressModel_->sortItems(logicalIndex, false);
        else
            compressModel_->sortItems(logicalIndex, true);
    }
}

void LocalDirDockWidget::favoritesDirContextMenu()
{
    QMenu menu;
    QString currentFileName = dir();
    QList<FavoriteItem> items = dirFavorite->favoriteItems();
    bool isCurrent = false;
    foreach(auto const& item, items)
    {
        QAction* action = menu.addAction(item.caption, this, [&](bool)
        {
            setDir(item.fileName);
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
    menu.addAction(tr("Settings"), this, [=]{
        FavoriteSettingsDialog dialog;
        dialog.setDirFavorite(dirFavorite);
        dialog.exec();
    });
    menu.exec(QCursor::pos());
}

void LocalDirDockWidget::historyDirContextMenu()
{
    QStringList const& dirNames = dirHistory->dirs();
    if(dirNames.isEmpty())
        return;

    QMenu menu;
    QString currentDir = dir();

    foreach(auto const& dirName, dirNames)
    {
        QAction* action = menu.addAction(dirName, this, [&](bool){
            setDir(dirName);
        });

        if(dirName == currentDir)
        {
            action->setCheckable(true);
            action->setChecked(true);
        }
    }
    menu.exec(QCursor::pos());
}


void LocalDirDockWidget::customNormalContextMenu(const QPoint & pos)
{
    QModelIndex index = ui->tvNormal->indexAt(pos);

    QMenu menu;
    ContextMenuItems items;
    QString fileName;
    QFileInfo fileInfo;

    ContextMenuItems sendtoItems = ContextMenu::SendTo();
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
                cd(name);
            });
            QFont font = action->font();
            font.setBold(true);
            action->setFont(font);
            items = ContextMenu::DirCommands();
        }
    }

    foreach(auto const& item, items)
    {
        menu.addAction(item.icon, item.name, [=](bool) {
            QStringList fileNames = getSelectedFileNames(false, true);
            item.exec(fileNames);
        });
    }
    menu.addAction(tr("Copy File Path"), this, [=](bool) {
        QStringList fileNames = getSelectedFileNames(false, true);
        ClipBoard::copy(fileNames.join("\n"));
    });

    QMenu *sendTo = new QMenu(tr("Send to"));
    foreach(auto sendItem, sendtoItems)
    {
        sendTo->addAction(sendItem.icon, sendItem.name, this, [=](bool){
            QStringList fileNames = getSelectedFileNames();
            sendItem.exec(fileNames);
        });
    }

    menu.addSeparator();
    menu.addMenu(sendTo);
    menu.addSeparator();
    menu.addAction(tr("Cut"), this, SLOT(cut()));
    menu.addAction(tr("Copy"), this, SLOT(copy()));
    if(ClipBoard::canPaste())
        menu.addAction(tr("Paste"), this, [&](bool){
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
    menu.addAction(tr("Create Shortcut"), this, SLOT(createShortcut()));
    if(index.isValid())
    {
        if(!model_->isParent(index.row()))
        {
            menu.addAction(tr("Delete"), this, SLOT(delFiles()));
            menu.addAction(tr("Rename"), this, SLOT(rename()));
        }
    }
    else
    {
        menu.addAction(tr("New Folder"), this, [=](){ newFolder(); });
        menu.addAction(tr("New Txt File"), this, [=](){ newTxtFile(); });
    }
    menu.addSeparator();
    menu.addAction(tr("Properties"), this, [&](bool) {
        QStringList fileNames = getSelectedFileNames(false, true);
        if(fileNames.size() > 1)
            WinShell::Property(fileNames);
        else
            WinShell::Property(fileName);
    });
    menu.addSeparator();
    QPoint cursorPos = QCursor::pos();
    menu.addAction(tr("Show More Items"), this, [&](bool) {
        QStringList fileNames = getSelectedFileNames(false, true);
        ContextMenu::show(fileNames, (void *)winId(), cursorPos);
    });

    menu.exec(cursorPos);
}

void LocalDirDockWidget::customCompressContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->tvCompress->indexAt(pos);
    QMenu menu;
    if(!index.isValid())
    {
        QString fileName = compressModel_->compressFileName();
        QFileInfo fileInfo(fileName);
        QAction* action = menu.addAction(fileInfo.fileName(), this, [=]()
        {
            WinShell::Open(fileName);
        });
        QFont font = action->font();
        font.setBold(true);
        action->setFont(font);
        ContextMenuItems items = ContextMenu::FileCommands();
        foreach(auto const& item, items)
        {
            menu.addAction(item.icon, item.name, [=](bool){
                QStringList fileNames = getSelectedFileNames(false, true);
                item.exec(fileNames);
            });
        }
        menu.addAction(tr("Copy File Path"), this, [=](bool){
            ClipBoard::copy(fileName);
        });
        ContextMenuItems sendtoItems = ContextMenu::SendTo();
        QMenu *sendTo = new QMenu(tr("Send to"));
        foreach(auto sendItem, sendtoItems)
        {
            sendTo->addAction(sendItem.icon, sendItem.name, this, [=](bool){
                sendItem.exec(fileName);
            });
        }
        menu.addSeparator();
        menu.addMenu(sendTo);
        menu.addSeparator();
        menu.addAction(tr("New Folder"), this, [=](){ newFolder(); });
        menu.addAction(tr("New Txt File"), this, [=](){ newTxtFile(); });
        menu.addSeparator();
        menu.addAction(tr("Properties"), this, [=](){
            WinShell::Property(fileName);
        });
    }
    else
    {
        CompressFileInfo::Ptr fileInfo = compressModel_->fileInfo(index.row());
        if(fileInfo->isParent())
            return;

        if(fileInfo->isDir())
            menu.addAction(tr("Open"), this, [=](){
                if(compressModel_->cd(fileInfo->path()))
                    updateCurrentDir(compressModel_->dir());
                else
                {
                    setShowMode(Normal);
                    updateCurrentDir(model_->dir());
                }
            });
        else
        {
            menu.addAction(tr("View"), this, [=](){
                QDir targetDir = Utils::tempDir();
                if(compressModel_->extract(targetDir.path(), QStringList() << fileInfo->filePath(), false))
                {
                    QString fileName = targetDir.filePath(fileInfo->fileName());
                    FileNames::MakeFileNameAsParams(fileName);
                    WinShell::Exec(Utils::viewApp(), fileName);
                }
            });
            menu.addAction(tr("Edit"), this, [=](){
                QDir targetDir = Utils::tempDir();
                if(compressModel_->extract(targetDir.path(), QStringList() << fileInfo->filePath(), false))
                {
                    QString fileName = targetDir.filePath(fileInfo->fileName());
                    FileNames::MakeFileNameAsParams(fileName);
                    WinShell::Exec(Utils::editApp(), fileName);
                }
            });
        }
        menu.addSeparator();
        menu.addAction(tr("Copy to..."), this, [=]() {
            emit copyRequested();
        });
        menu.addAction(tr("Move to..."), this, [=](){
            emit moveRequested();
        });
        menu.addAction(tr("Rename"), this, [=](){
            QModelIndex nameIndex = compressModel_->index(index.row(), 0);
            ui->tvCompress->edit(nameIndex);
        });
        menu.addAction(tr("Delete"), this, [=](){
            compressModel_->rm(fileInfo->filePath());
            compressModel_->refresh();
        });
        menu.addSeparator();
        menu.addAction(tr("New Folder"), this, [=](){ newFolder(); });
        menu.addAction(tr("New Txt File"), this, [=](){ newTxtFile(); });
        menu.addSeparator();
        menu.addAction(tr("Properties"), this, [=](){
            PropertyDialog dialog;
            dialog.setFileInfo(compressModel_->fileInfo(index.row()));
            dialog.exec();
        });
    }

    menu.exec(QCursor::pos());
}

void LocalDirDockWidget::normalBeginDragFile(QPoint const& point)
{
    QModelIndex index = ui->tvNormal->indexAt(point);
    if(!index.isValid())
        return;

    QDrag *drag = new QDrag(ui->tvNormal);
    QStringList fileNames = ClipBoard::fileNames(getSelectedFileNames());
    QMimeData* mimeData = WinShell::dropMimeData(fileNames);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap(":/image/copy.png"));
    drag->exec(Qt::LinkAction | Qt::MoveAction | Qt::CopyAction , Qt::CopyAction);
}

void LocalDirDockWidget::compressBeginDragFile(QPoint const& point)
{
    QModelIndex index = ui->tvCompress->indexAt(point);
    if(!index.isValid())
        return;

    QDrag *drag = new QDrag(ui->tvCompress);
    QStringList fileNames = selectedCompressedFileNames();
    QMimeData* mimeData = new QMimeData();

    mimeData->setText(fileNames.join("\n"));
    mimeData->setData("compressedFileName",
                      compressModel_->compressFileName().toUtf8());
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap(":/image/copy.png"));
    drag->exec(Qt::CopyAction);
}

void LocalDirDockWidget::normalDragEnter(QDragEnterEvent * event)
{
    QMimeData const* mimeData = event->mimeData();
    if(mimeData)
        event->acceptProposedAction();
}

void LocalDirDockWidget::compressDragEnter(QDragEnterEvent * event)
{
    QMimeData const* mimeData = event->mimeData();
    if(mimeData)
        event->acceptProposedAction();
}

void LocalDirDockWidget::normalDragMove(QDragMoveEvent * event)
{
    QModelIndex index = ui->tvNormal->indexAt(event->pos());
    bool isSelf = (event->source() == ui->tvNormal);
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

void LocalDirDockWidget::compressDragMove(QDragMoveEvent * event)
{
    QModelIndex index = ui->tvCompress->indexAt(event->pos());
    bool isSelf = (event->source() == ui->tvCompress);
    if(index.isValid())
    {
        if(isSelf && compressModel_->fileInfo(index.row())->isFile())
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

void LocalDirDockWidget::normalDrop(QDropEvent * event)
{
    QModelIndex index = ui->tvNormal->indexAt(event->pos());
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
        if(!Utils::question(QString(tr("Move %1 files or folders to\n%2"))
                            .arg(fileNames.size()).arg(filePath)))
            return;
        fileTransfer(FileNames::GetFileNames(fileNames, filePath), true);
    }
    else if(event->dropAction() == Qt::CopyAction)
    {
        if(!Utils::question(QString(tr("Copy %1 files or folders to\n%2"))
                            .arg(fileNames.size()).arg(filePath)))
            return;
        QString remoteSrc = ClipBoard::remoteSrc(mimeData);
        if(remoteSrc.isEmpty())
        {
            QString compressedFileName = ClipBoard::compressedFileName(mimeData);
            if(compressedFileName.isEmpty())
                fileTransfer(FileNames::GetFileNames(fileNames, filePath), false);
            else
                emit compressFileExtract(fileNames);
        }
        else
            emit remoteDownload(remoteSrc, fileNames, filePath);
    }
    else if(event->dropAction() == Qt::LinkAction)
    {
        if(!Utils::question(QString(tr("Create %1 shortcuts in %2"))
                            .arg(fileNames.size()).arg(filePath)))
            return;
        foreach(auto const& fileName,  fileNames)
        {
            WinShell::CreateShortcut(QString("%1 - shortcut.lnk")
                                     .arg(fileName), fileName);
        }
    }
    model_->refresh();
}

void LocalDirDockWidget::compressDrop(QDropEvent * event)
{
    QModelIndex index = ui->tvCompress->indexAt(event->pos());
    QString filePath;
    QString subFilePath;
    if(!index.isValid())
        filePath = compressModel_->dir();
    else
    {
        if(compressModel_->fileInfo(index.row())->isFile())
            filePath = compressModel_->dir();
        else
        {
            subFilePath = compressModel_->fileName(index.row());
            filePath = QString("%1\\%2").arg(compressModel_->dir(), subFilePath);
        }
    }

    QMimeData const* mimeData = event->mimeData();
    if(!mimeData)
        return;
    QStringList fileNames = ClipBoard::fileNames(mimeData);
    if(!Utils::question(QString(tr("Copy %1 files or folders to\n%2"))
                        .arg(fileNames.size()).arg(filePath)))
        return;
    compressFiles(fileNames, subFilePath);
    compressModel_->refresh();
}

void LocalDirDockWidget::cut()
{
    QStringList fileNames = getSelectedFileNames(false, true);
    WinShell::Copy(fileNames, true);
}

void LocalDirDockWidget::copy()
{
    QStringList fileNames = getSelectedFileNames(false, true);
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

void LocalDirDockWidget::deleteFiles(bool isPrompt)
{
    if(!isPrompt)
    {
        if(ui->tvNormal->isVisible())
            delFiles();
        else
        {
            compressModel_->rm(selectedCompressedFileNames());
            compressModel_->refresh();
        }
        return;
    }

    QStringList fileNames;
    if(ui->tvNormal->isVisible())
        fileNames = getSelectedFileNames(true);
    else
        fileNames = selectedCompressedFileNames();
    QString tipText;
    if(fileNames.size() > 1)
        tipText = QString(tr("Are you sure you want to delete %1 files or folders?\n\n%2"))
                .arg(fileNames.size())
                .arg(fileNames.join("\n"));
    else if(fileNames.size() > 0)
    {
        QModelIndex index;
        QString type;
        if(ui->tvNormal->isVisible())
        {
            index = ui->tvNormal->currentIndex();
            type = model_->isDir(index.row()) ? tr("folder") : tr("file");
        }
        else
        {
            index = ui->tvCompress->currentIndex();
            type = compressModel_->isDir(index.row()) ? tr("folder") : tr("file");
        }
        tipText = QString(tr("Are you sure you want to delete the %1 %2?"))
                .arg(type, fileNames.first());
    }
    if(tipText.isEmpty())
    {
        Utils::warring(tr("No files or folders selected!"));
        return;
    }
    if(Utils::question(tipText))
    {
        if(ui->tvNormal->isVisible())
            delFiles();
        else
        {
            compressModel_->rm(fileNames);
            compressModel_->refresh();
        }
    }
}

void LocalDirDockWidget::delFiles()
{
    QStringList fileNames = getSelectedFileNames();
    FileManager fileManger;
    fileManger.delereFiles(fileNames);
    model_->refresh();
}

void LocalDirDockWidget::rename()
{
    QString fileName = selectedFileName(true);
    if(fileName.isEmpty())
        return;

    QModelIndex index = ui->tvNormal->currentIndex();
    QModelIndex nameIndex = model_->index(index.row(), 0);
    ui->tvNormal->edit(nameIndex);
}

void LocalDirDockWidget::createShortcut()
{
    QStringList fileNames = getSelectedFileNames(false, true);
    foreach(auto const& fileName,  fileNames)
    {
        WinShell::CreateShortcut(QString("%1 - shortcut.lnk")
                                 .arg(fileName), fileName);
    }
    model_->refresh();
}

void LocalDirDockWidget::newFolder()
{
    QString path = Utils::getText(tr("Folder Name"));
    if(path.isEmpty())
        return;
    if(ui->tvNormal->isVisible())
    {
        if(model_->mkdirs(path))
            model_->refresh();
    }
    else
    {
        if(compressModel_->mkdir(path))
            compressModel_->refresh();
    }
}

void LocalDirDockWidget::viewFile()
{
    if(ui->tvNormal->isVisible())
    {
        QString fileName = selectedFileName();
        QFileInfo fileInfo(fileName);

        if(fileInfo.isFile())
        {
            FileNames::MakeFileNameAsParams(fileName);
            WinShell::Exec(Utils::viewApp(), fileName);
        }
    }
    else
    {
        CompressFileInfo::Ptr fileInfo = selectedCompressedFileName();
        if(!fileInfo || fileInfo->isDir())
            return;

        QDir targetDir = Utils::tempDir();
        if(compressModel_->extract(targetDir.path(),
                                   QStringList() << fileInfo->filePath(),
                                   false))
        {
            QString fileName = targetDir.filePath(fileInfo->fileName());
            FileNames::MakeFileNameAsParams(fileName);
            WinShell::Exec(Utils::viewApp(), fileName);
        }
    }
}
void LocalDirDockWidget::editFile()
{
    if(ui->tvNormal->isVisible())
    {
        QString fileName = selectedFileName();
        QFileInfo fileInfo(fileName);
        if(fileInfo.isFile())
        {
            FileNames::MakeFileNameAsParams(fileName);
            WinShell::Exec(Utils::editApp(), fileName);
        }
    }
    else
    {
        CompressFileInfo::Ptr fileInfo = selectedCompressedFileName();
        if(!fileInfo || fileInfo->isDir())
            return;
        QDir targetDir = Utils::tempDir();
        if(compressModel_->extract(targetDir.path(),
                                   QStringList() << fileInfo->filePath(),
                                   false))
        {
            QString fileName = targetDir.filePath(fileInfo->fileName());
            FileNames::MakeFileNameAsParams(fileName);
            WinShell::Exec(Utils::editApp(), fileName);
        }
    }
}

void LocalDirDockWidget::copyFiles(QString const& dstFilePath)
{
    QStringList fileNames;
    if(showMode() == Compress)
        fileNames = selectedCompressedFileNames();
    else
        fileNames = getSelectedFileNames(true);

    if(fileNames.isEmpty())
    {
        Utils::warring(tr("No files or folders selected!"));
        return;
    }

    FileOperateConfirmDialog dialog;
    if(fileNames.size() > 1)
        dialog.setLabel(QString(tr("Copy %1 files or folders to:"))
                        .arg(fileNames.size()));
    else
    {
        QFileInfo fileInfo(selectedFileName());
        QString type;
        if(fileInfo.isDir())
            type = tr("folder");
        else
            type = tr("file");
        dialog.setLabel(QString(tr("Copy the %1 %2 to:"))
                        .arg(type, fileNames.first()));
    }
    dialog.setPath(dstFilePath);
    if(dialog.exec() == QDialog::Accepted)
    {
        if(showMode() == Compress)
            extractFiles(fileNames, dialog.path(), false);
        else
            copyFilels(getSelectedFileNames(), dialog.path());
    }
}

void LocalDirDockWidget::moveFiles(QString const& dstFilePath)
{
    QStringList fileNames;
    if(showMode() == Compress)
        fileNames = selectedCompressedFileNames();
    else
        fileNames = getSelectedFileNames(true);

    if(fileNames.isEmpty())
    {
        Utils::warring(tr("No files or folders selected!"));
        return;
    }
    FileOperateConfirmDialog dialog;
    if(fileNames.size() > 1)
        dialog.setLabel(QString(tr("Move %1 files or folders to:"))
                        .arg(fileNames.size()));
    else
    {
        QFileInfo fileInfo(selectedFileName());
        QString type;
        if(fileInfo.isDir())
            type = tr("folder");
        else
            type = tr("file");
        dialog.setLabel(QString(tr("Move the %1 %2 to:"))
                        .arg(type, fileNames.first()));
    }
    dialog.setPath(dstFilePath);
    if(dialog.exec() == QDialog::Accepted)
    {
        if(showMode() == Compress)
        {
            extractFiles(fileNames, dialog.path(), true);
            compressModel_->refresh();
        }
        else if(showMode() == Normal)
        {
            cutFiles(getSelectedFileNames(), dialog.path());
            model_->refresh();
        }
    }
}

void LocalDirDockWidget::refresh()
{
    if(showMode() == Normal)
        model_->refresh();
    else if(showMode() == Compress)
        compressModel_->refresh();
}

void LocalDirDockWidget::selectAll()
{
    if(showMode() == Normal)
    {
        ui->tvNormal->selectAll();
        if(model_->isParent(0))
        {
            QModelIndexList indexList = ui->tvNormal->selectionModel()->selectedColumns(0);
            foreach(auto const& index, indexList)
                ui->tvNormal->selectionModel()->select(index,
                                                       QItemSelectionModel::Deselect);
        }
    }
    else if(showMode() == Compress)
    {
        ui->tvCompress->selectAll();
        QModelIndexList indexList = ui->tvCompress->selectionModel()->selectedColumns(0);
        foreach(auto const& index, indexList)
            ui->tvCompress->selectionModel()->select(index,
                                                   QItemSelectionModel::Deselect);
    }
}

void LocalDirDockWidget::compressFiles(QString const& dstFilePath)
{
    QStringList fileNames = getSelectedFileNames();
    if(fileNames.isEmpty())
    {
        Utils::warring(tr("No select files!"));
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
            QFileInfo fileInfo(fileNames[0]);
            if(fileInfo.isDir())
                fileName = fileInfo.baseName() + CompressConfirmDialog::CurrentSuffix();
            else
                fileName = fileInfo.completeBaseName() + CompressConfirmDialog::CurrentSuffix();
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

            connect(&compresser, &FileCompresser::progress,
                    &dialog, &FileProgressDialog::progressText);
            connect(&compresser, &FileCompresser::finished,
                    &dialog, &FileProgressDialog::finished);
            connect(&compresser, &FileCompresser::error,
                    &dialog, &FileProgressDialog::error);

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
    QStringList fileNames = getSelectedFileNames();
    if(fileNames.isEmpty())
    {
        Utils::warring("No select files!");
        return;
    }

    QString fileName;
    if(!FileUncompresser::isCompressFiles(fileNames, fileName))
        Utils::warring(QString(tr("Compression package error!\n%1"))
                       .arg(fileName));
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
            {
                param.password = Utils::getPassword(tr("Input password"));
                if(param.password.isEmpty())
                    return;
            }

            connect(&uncompresser, &FileUncompresser::progress,
                    &dialog, &FileProgressDialog::progressText);
            connect(&uncompresser, &FileUncompresser::finished,
                    &dialog, &FileProgressDialog::finished);
            connect(&uncompresser, &FileUncompresser::error,
                    &dialog, &FileProgressDialog::error);

            dialog.setModal(true);
            dialog.show();

            uncompresser.uncompress(fileNames, param, targetPath,
                                    FileUncompresser::isMultiVol(fileNames));
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

void LocalDirDockWidget::extractFiles(QStringList const& fileNames,
                                      QString const& targetPath)
{
    extractFiles(fileNames, targetPath, false);
}

void LocalDirDockWidget::searchFiles(QString const& dstFilePath)
{
    SearchFileDialog dialog;
    dialog.setSearchPath(dstFilePath);
    connect(&dialog, &SearchFileDialog::viewFile,
            this, [=](QString const& fileName)
    {
        QFileInfo fileInfo(fileName);
        if(fileInfo.isFile())
            WinShell::Exec(Utils::viewApp(), fileName);
    });
    connect(&dialog, &SearchFileDialog::goToFile,
            this, [=](QString const& fileName)
    {
        goToFile(fileName);
    });
    dialog.exec();
}

void LocalDirDockWidget::execCommand(QString const& command)
{
    QStringList args = command.split(" ");
    if(args.size() < 2)
    {
        WinShell::Run(command, QString(), model_->dir());
    }
    else
    {
        if(args[0] == "cd")
            cd(args[1]);
        else
            WinShell::Run(args[0], args.mid(1).join(" "), model_->dir());
    }
}

void LocalDirDockWidget::newTxtFile()
{
    QString fileName = Utils::getText(tr("File Name"), "*.txt");
    if(fileName.isEmpty())
        return;
    if(ui->tvNormal->isVisible())
    {
        QFile file(model_->filePath(fileName));
        if(file.open(QIODevice::WriteOnly))
        {
            file.close();
            model_->refresh();
        }
    }
    else
    {
        QString filePath = Utils::tempDir().filePath(fileName);
        QFile file(filePath);
        if(file.open(QIODevice::WriteOnly))
            file.close();
        if(compressModel_->add(filePath, true))
            compressModel_->refresh();
    }
}

bool LocalDirDockWidget::isMultiSelected()
{
    return ui->tvNormal->selectionModel()->selectedRows(0).size() > 1;
}

bool LocalDirDockWidget::isCompressFiles(QString const& suffix)
{
    QStringList suffixs = QStringList() << "zip" << "7z" << "wim" << "tar"
                                        << "gz" << "xz" << "bz2" << "iso"
                                        << "rar";
    return suffixs.contains(suffix.toLower());
}

QStringList LocalDirDockWidget::getSelectedFileNames(bool isOnlyFilename,
                                                  bool isParent) const
{
    QModelIndexList indexs = ui->tvNormal->selectionModel()->selectedRows(0);
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

QStringList LocalDirDockWidget::selectedCompressedFileNames()
{
    QModelIndexList indexs = ui->tvCompress->selectionModel()->selectedRows(0);
    QStringList names;
    for(int i = 0; i < indexs.size(); i++)
    {
        if(compressModel_->isParent(indexs[i].row()))
            continue;
         names << compressModel_->filePath(indexs[i].row());
    }
    return names;
}

QString LocalDirDockWidget::selectedFileName(bool isOnlyFilename) const
{
    QModelIndex index = ui->tvNormal->currentIndex();
    if(!index.isValid())
        return QString();
    if(isOnlyFilename)
        return model_->fileName(index.row());
    return model_->filePath(index.row());
}

CompressFileInfo::Ptr LocalDirDockWidget::selectedCompressedFileName()
{
    QModelIndex index = ui->tvCompress->currentIndex();
    if(!index.isValid())
        return CompressFileInfo::Ptr();

    return compressModel_->fileInfo(index.row());
}

void LocalDirDockWidget::copyFilels(QStringList const& fileNames,
                                    QString const& dstFilePath)
{
    fileTransfer(FileNames::GetFileNames(fileNames, dstFilePath), false);
}

void LocalDirDockWidget::cutFiles(QStringList const& fileNames,
                                  QString const& dstFilePath)
{
    fileTransfer(FileNames::GetFileNames(fileNames, dstFilePath), true);
}

void LocalDirDockWidget::fileTransfer(FileNames const& fileNames,
                                      bool isMove)
{
    FileTransfer transfer;
    FileProgressDialog dialog(this);

    connect(&transfer, &FileTransfer::totalProgress,
            &dialog, &FileProgressDialog::totalProgress);
    connect(&transfer, &FileTransfer::fileProgress,
            &dialog, &FileProgressDialog::fileProgress);
    connect(&transfer, &FileTransfer::finished,
            &dialog, &FileProgressDialog::finished);
    connect(&transfer, &FileTransfer::error,
            &dialog, &FileProgressDialog::error);

    dialog.setModal(true);
    if(fileNames.size() == 1)
        dialog.hideTotalProgress();
    dialog.show();
    if(isMove)
    {
        dialog.setWindowTitle(tr("MoveFiles"));
        transfer.moveFiles(fileNames);
    }
    else
    {
        dialog.setWindowTitle(tr("CopyFiles"));
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

void LocalDirDockWidget::extractFiles(QStringList const& fileNames,
                                      QString const& targetPath,
                                      bool isMove)
{
    FileUncompresser uncompresser;
    FileProgressDialog dialog(this);
    dialog.setStatusTextMode();

    connect(&uncompresser, &FileUncompresser::progress,
            &dialog, &FileProgressDialog::progressText);
    connect(&uncompresser, &FileUncompresser::finished,
            &dialog, &FileProgressDialog::finished);
    connect(&uncompresser, &FileUncompresser::error,
            &dialog, &FileProgressDialog::error);

    dialog.setModal(true);
    dialog.show();

    uncompresser.extract(compressModel_->compressFileName(),
                         targetPath,
                         fileNames,
                         true);
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
    if(!dialog.isCancel() && isMove)
       uncompresser.remove(compressModel_->compressFileName(), fileNames);
}

void LocalDirDockWidget::compressFiles(QStringList const& fileNames,
                                       QString const& filePath)
{
    FileCompresser compresser;
    FileProgressDialog dialog(this);
    dialog.setStatusTextMode();

    connect(&compresser, &FileCompresser::progress,
            &dialog, &FileProgressDialog::progressText);
    connect(&compresser, &FileCompresser::finished,
            &dialog, &FileProgressDialog::finished);
    connect(&compresser, &FileCompresser::error,
            &dialog, &FileProgressDialog::error);

    dialog.setModal(true);
    dialog.show();

    compresser.update(fileNames, compressModel_->compressFileName(), false);
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
    if(!dialog.isCancel())
        compressModel_->rename(fileNames, filePath);
}

void LocalDirDockWidget::goToFile(QString const& fileName)
{
    QFileInfo fileInfo(fileName);
    QString filePath = fileInfo.dir().path();
    QString baseName = fileInfo.fileName();

    if(model_->dir() != filePath)
        setDir(filePath);

    int index = model_->indexOfFile(baseName);
    if(index != -1)
    {
        for(int col = 0; col < model_->columnCount(); col++)
        {
            QModelIndex modeIndex = model_->index(index, col);
            ui->tvNormal->selectionModel()->select(modeIndex,
                                                   QItemSelectionModel::Select);
        }
    }
    emit statusTextChanged(getStatusText());
}

QString LocalDirDockWidget::getStatusText()
{
    if(ui->tvNormal->isVisible())
        return getStatusText(ui->tvNormal->selectionModel(),
                             model_);
    else
        return getStatusText(ui->tvCompress->selectionModel(),
                             compressModel_);
}

QString LocalDirDockWidget::getStatusText(QItemSelectionModel* selectMode,
                                          DirModel* model)
{
    QModelIndexList indexs = selectMode->selectedRows(0);
    QStringList names;
    int files = 0;
    int dirs = 0;
    qint64 fileSize = 0;
    for(int i = 0; i < indexs.size(); i++)
    {
        if(model->isParent(indexs[i].row()))
            continue;
        if(model->isDir(indexs[i].row()))
            dirs++;
        else
        {
            files++;
            fileSize += model->fileSize(indexs[i].row());
        }
    }
    return  QString(tr("%1/%2,%3/%4 files,%5/%6 dir(s)"))
            .arg(Utils::formatFileSizeKB(fileSize),
                 Utils::formatFileSizeKB(model->fileSizes()))
            .arg(files).arg(model->fileCount())
            .arg(dirs).arg(model->dirCount());
}

void LocalDirDockWidget::updateCurrentDir(QString const& dir,
                                          QString const& caption,
                                          bool isNavigation)
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

void LocalDirDockWidget::setShowMode(ShowMode const& mode)
{
    showMode_ = mode;
    ui->tvNormal->setVisible(showMode_ == Normal);
    ui->tvCompress->setVisible(showMode_ == Compress);
}
