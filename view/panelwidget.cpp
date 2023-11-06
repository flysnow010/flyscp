#include "panelwidget.h"
#include "ui_panelwidget.h"
#include "view/localdirdockwidget.h"
#include "util/utils.h"
#include "core/winshell.h"

#include <QDir>
#include <QHBoxLayout>
#include <QToolButton>
#include <QFileIconProvider>
#include <QButtonGroup>
#include <QSettings>
#include <QMenu>
#include <QLabel>
#include <QStorageInfo>

#include <climits>

PanelWidget::PanelWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PanelWidget)
    , buttonGroup(new QButtonGroup(this))
    , labelDiskInfo(new QLabel(this))
    , isShowTips_(true)
    , isGotRoot_(false)
{
    ui->setupUi(this);
    ui->tabWidget->setTabBarAutoHide(true);
    initDrivers();
    connect(buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(buttonClicked(QAbstractButton*)));
    connect(ui->tabWidget, &QTabWidget::currentChanged,
            this, &PanelWidget::currentChanged);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested,
            this, &PanelWidget::tabCloseRequested);
}

PanelWidget::~PanelWidget()
{
    delete ui;
}

void PanelWidget::addDirTab(QWidget* widget,
                            QIcon const& icon,
                            QString const& text)
{
    ui->tabWidget->addTab(widget, icon, text);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
    emit tabCountChanged(ui->tabWidget->count());
}

void PanelWidget::setTabBarAutoHide(int count)
{
    ui->tabWidget->setTabBarAutoHide(count > 1 ? false : true);
}

bool PanelWidget::isRemote() const
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        return dir->isRemote();
    return  false;
}

QString PanelWidget::currentDir() const
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        return dir->dir();
    return  QString();
}

QStringList PanelWidget::selectedFileNames() const
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        return dir->selectedFileNames();
    return  QStringList();
}

void PanelWidget::preDir()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(!dir)
        return;

    dir->preDir();
}

void PanelWidget::nextDir()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(!dir)
        return;

    dir->nextDir();
}

void PanelWidget::upddateDrive()
{
    updateTexts(ui->tabWidget->currentWidget());
}

void PanelWidget::libDirContextMenu()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(!dir)
        return;

    QMenu menu;

    ShellMenuItems menuItems = WinShell::shellMenuItems();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    foreach(auto const& menuItem, menuItems)
    {
        if(menuItem.isDir())
            menu.addAction(menuItem.icon, menuItem.caption,
                           this, [=](bool)
            {
                dir->setDir(Utils::toLinuxPath(menuItem.filePath),
                            menuItem.showPath());
            });
        else
        {
            QMenu* subMenu = new QMenu(menuItem.caption);
            QAction* menuAction = subMenu->menuAction();

            menuAction->setIcon(menuItem.icon);
            connect(menuAction,  &QAction::triggered,
                    this, [=](){ menuItem.exec(); });
            connect(menuAction,  &QAction::hovered,
                    this, [=]()
            {
                if(subMenu->isEmpty())
                {
                    ShellMenuItems childMenuItems;
                    WinShell::shellSubMenuItems(menuItem, childMenuItems);
                    if(childMenuItems.isEmpty())
                        menuAction->setMenu(0);
                    foreach(auto const& childMenuItem, childMenuItems)
                    {
                        if(childMenuItem.isDir())
                            subMenu->addAction(childMenuItem.icon, childMenuItem.caption,
                                               this, [=](bool){
                                dir->setDir(Utils::toLinuxPath(childMenuItem.filePath),
                                            childMenuItem.showPath());
                            });
                        else
                            subMenu->addAction(childMenuItem.icon, childMenuItem.caption,
                                               this, [=](bool) {
                                childMenuItem.exec();
                            });
                    }
                }
            });
            menu.addAction(menuAction);
        }
    }

    QApplication::restoreOverrideCursor();
    menu.exec(QCursor::pos());
}

void PanelWidget::updateTexts(QWidget* widget)
{
    BaseDir* dir = dynamic_cast<BaseDir *>(widget);
    if(dir)
    {
        QList<QAbstractButton*> buttons = buttonGroup->buttons();
        QString filePath = dir->dir().toUpper();
        for(int i = 0; i < buttons.size(); i++)
        {
            if(filePath.startsWith(buttons[i]->text().toUpper()))
            {
                buttons[i]->setChecked(true);
                QStorageInfo storeInfo(QString("%1:/").arg(buttons[i]->text()));
                QString diskInfo = QString(tr("[%1] %2 available, %3 in total"))
                        .arg(buttons[i]->toolTip(),
                             Utils::formatFileSize(storeInfo.bytesFree()),
                             Utils::formatFileSize(storeInfo.bytesTotal()));
                labelDiskInfo->setText(diskInfo);
                break;
            }
        }
    }
}

void PanelWidget::initDrivers()
{
    QHBoxLayout* layout = new QHBoxLayout();

    layout->setMargin(3);
    layout->setSpacing(5);
    for(char ch = 'a'; ch <= 'z'; ch++)
    {
        QToolButton* button = new QToolButton();
        button->setCheckable(true);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(QString::fromLocal8Bit(&ch, 1));
        button->setIconSize(QSize(10, 10));
        button->installEventFilter(this);
        layout->addWidget(button);
        buttonGroup->addButton(button, QChar(ch).unicode());
        button->hide();
    }

    QFileIconProvider fip;
    ShellMenuItems drivers = WinShell::computerShellItems();

    for(int i = 0; i < drivers.size(); i++)
    {
        if(drivers[i].isDrive())
        {
            QChar ch = drivers[i].filePath.at(0).toLower();
            QAbstractButton* button = buttonGroup->button(ch.unicode());
            button->setIcon(fip.icon(QFileInfo(drivers[i].filePath)));
            button->setToolTip(drivers[i].showToolTip());
            button->show();
        }
    }

    QToolButton* homeButton = new QToolButton();
    homeButton->setIcon(QIcon(":/image/home.png"));
    layout->addWidget(homeButton);

    QToolButton* rootButton = new QToolButton();
    rootButton->setText("\\");
    layout->addWidget(rootButton);

    QToolButton *topButton = new QToolButton();
    topButton->setText("..");
    layout->addWidget(topButton);
    layout->addWidget(labelDiskInfo);
    layout->addStretch();

    connect(homeButton, SIGNAL(clicked()), this, SLOT(backToHome()));
    connect(rootButton, SIGNAL(clicked()), this, SLOT(backToRoot()));
    connect(topButton, SIGNAL(clicked()), this, SLOT(backToPrePath()));
    ui->driverWidget->setLayout(layout);
}

void PanelWidget::updateDrivers(bool isAdded)
{
    ShellMenuItems drivers = WinShell::computerShellItems();
    if(isAdded)
    {
        QFileIconProvider fip;
        for(int i = 0; i < drivers.size(); i++)
        {
            if(drivers[i].isDrive())
            {
                QChar ch = drivers[i].filePath.at(0).toLower();
                QAbstractButton* button = buttonGroup->button(ch.unicode());
                if(button && button->isHidden())
                {
                    button->setIcon(fip.icon(QFileInfo(drivers[i].filePath)));
                    button->setToolTip(drivers[i].showToolTip());
                    button->show();
                }
            }
        }
    }
    else
    {
        QList<int> ids;
        for(int i = 0; i < drivers.size(); i++)
            ids << drivers[i].filePath.at(0).toLower().unicode();
        QList<QAbstractButton*> buttons = buttonGroup->buttons();
        foreach(auto & button, buttons)
        {
            int id = buttonGroup->id(button);
            if(!ids.contains(id))
                button->hide();
        }
    }
}

int PanelWidget::tabCount() const
{
    return ui->tabWidget->count();
}

void PanelWidget::showDriveButtons(bool isShow)
{
    ui->driverWidget->setVisible(isShow);
}

#define CALL_FUNCTION(function, param) \
    for(int i = 0; i < ui->tabWidget->count(); i++){ \
        BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->widget(i)); \
        if(dir) \
            dir->function(param); \
    }

void PanelWidget::showHeader(bool isShow)
{
    CALL_FUNCTION(showHeader, isShow)
}

void PanelWidget::showCurrentDir(bool isShow)
{
    CALL_FUNCTION(showCurrentDir, isShow)
}

void PanelWidget::showDeskNavigationButton(bool isShow)
{
    CALL_FUNCTION(showDeskNavigationButton, isShow)
}

void PanelWidget::showFavoriteButton(bool isShow)
{
    CALL_FUNCTION(showFavoriteButton, isShow)
}

void PanelWidget::showHistoryButton(bool isShow)
{
    CALL_FUNCTION(showHistoryButton, isShow)
}

void PanelWidget::showHiddenAndSystem(bool isShow)
{
    CALL_FUNCTION(showHiddenAndSystem, isShow)
}

void PanelWidget::showToolTips(bool isShow)
{
    CALL_FUNCTION(showToolTips, isShow)
}

void PanelWidget::showDriveToolTips(bool isShow)
{
    isShowTips_ = isShow;
}

void PanelWidget::showParentInRoot(bool isShow)
{
    CALL_FUNCTION(showParentInRoot, isShow)
}

void PanelWidget::setDirSortByTime(bool isOn)
{
    CALL_FUNCTION(setDirSortByTime, isOn)
}

void PanelWidget::setRenameFileName(bool isOn)
{
    CALL_FUNCTION(setRenameFileName, isOn)
}

void PanelWidget::showAllIconWithExeAndLink(bool isShow)
{
    CALL_FUNCTION(showAllIconWithExeAndLink, isShow)
}

void PanelWidget::showAllIcon(bool isShow)
{
    CALL_FUNCTION(showAllIcon, isShow)
}

void PanelWidget::showStandardIcon(bool isShow)
{
    CALL_FUNCTION(showStandardIcon, isShow)
}

void PanelWidget::showNoneIcon(bool isShow)
{
    CALL_FUNCTION(showNoneIcon, isShow)
}

void PanelWidget::showIconForFyleSystem(bool isShow)
{
    CALL_FUNCTION(showIconForFyleSystem, isShow)
}

void PanelWidget::showIconForVirtualFolder(bool isShow)
{
    CALL_FUNCTION(showIconForVirtualFolder, isShow)
}

void PanelWidget::showOverlayIcon(bool isShow)
{
    CALL_FUNCTION(showOverlayIcon, isShow)
}

void PanelWidget::fileIconSize(int size)
{
    CALL_FUNCTION(fileIconSize, size)
}

void PanelWidget::fileFont(QFont const& font)
{
    CALL_FUNCTION(fileFont, font)
}

void PanelWidget::setDriveFont(QFont const& font)
{
    QList<QWidget *> children = ui->driverWidget->findChildren<QWidget *>();
    foreach(auto child, children)
    {
        child->setFont(font);
    }
}

void PanelWidget::setItemColor(QString const& fore,
                  QString const& back,
                  QString const&alternate)
{
    for(int i = 0; i < ui->tabWidget->count(); i++)
    {
        BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->widget(i));
        if(dir)
            dir->setItemColor(fore, back, alternate);
    }
}

void PanelWidget::setItemSelectedColor(QString const& back,
                  QString const& mark,
                  QString const&cursor)
{
    for(int i = 0; i < ui->tabWidget->count(); i++)
    {
        BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->widget(i));
        if(dir)
            dir->setItemSelectedColor(back, mark, cursor);
    }
}

void PanelWidget::setUnActived()
{
    for(int i = 0; i < ui->tabWidget->count(); i++)
    {
        BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->widget(i));
        if(dir)
        {
            if(dir->isActived())
                dir->setActived(false);
        }
    }
}

void PanelWidget::refresh()
{
    for(int i = 0; i < ui->tabWidget->count(); i++)
    {
        BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->widget(i));
        if(dir)
            dir->refresh();
    }
}

void PanelWidget::execCommand(QString const& command)
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->execCommand(command);
}

void PanelWidget::viewFile()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->viewFile();
}

void PanelWidget::newFolder()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->newFolder();
}

void PanelWidget::newTxtFile()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->newTxtFile();
}

void PanelWidget::deleteFiles(bool isPrompt)
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->deleteFiles(isPrompt);
}

void PanelWidget::selectAll()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->selectAll();
}

void PanelWidget::uploadFiles(QStringList const& fileNames)
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->uploadFiles(fileNames);
}

void PanelWidget::copyFiles(QString const& dstFilePath)
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->copyFiles(dstFilePath);
}
void PanelWidget::moveFiles(QString const& dstFilePath)
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
     if(dir)
         dir->moveFiles(dstFilePath);
}

void PanelWidget::searchFiles(QString const& dstFilePath)
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->searchFiles(dstFilePath);
}

bool PanelWidget::isActived() const
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        return dir->isActived();
    return false;
}

void PanelWidget::refreshCurrent()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->refresh();
}

void PanelWidget::retranslateUi(QString const& tabText)
{
    ui->retranslateUi(this);
    ui->tabWidget->setTabText(0, tabText);
    updateTexts(ui->tabWidget->currentWidget());
    for(int i = 0; i < ui->tabWidget->count(); i++)
    {
        BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->widget(i));
        if(dir)
            dir->retranslateUi();
    }
}

void PanelWidget::buttonClicked(QAbstractButton* button)
{
    updateDir(button->text().toUpper());
}

void PanelWidget::backToHome()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->setDir(dir->home());
}

void PanelWidget::backToRoot()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->setDir(dir->root());
}

void PanelWidget::backToPrePath()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
        dir->cd("..");
}

void PanelWidget::updateDir(QString const& driver)
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(dir)
    {
        if(isGotRoot_)
            dir->setDir(QString("%1:/").arg(driver));
        else
            dir->setDir(dir->findDir(driver));
    }
}

void PanelWidget::currentChanged(int index)
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->widget(index));
    if(!dir)
        return;
     QList<QAbstractButton*> buttons = buttonGroup->buttons();
     emit tabActived(dir->dir());
     dir->setActived(true);
     foreach(auto button, buttons)
     {
         button->setEnabled(!dir->isRemote());
     }
}

void PanelWidget::closeTab(QWidget *w)
{
   int index = ui->tabWidget->indexOf(w);
   if(index < 0)
       return;

   tabCloseRequested(index);
}

void PanelWidget::tabCloseRequested(int index)
{
    QWidget* w = ui->tabWidget->widget(index);
    BaseDir* dir = dynamic_cast<BaseDir *>(w);
    if(!dir || !dir->isRemote())
        return;

    w->deleteLater();
    ui->tabWidget->removeTab(index);
    emit tabCountChanged(ui->tabWidget->count());
}

bool PanelWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::ToolTip
            || event->type() == QEvent::StatusTip)
        return !isShowTips_;
    return QWidget::eventFilter(obj, event);
}
