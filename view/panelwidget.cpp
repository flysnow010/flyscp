#include "panelwidget.h"
#include "ui_panelwidget.h"
#include "view/localdirdockwidget.h"
#include "util/utils.h"
#include "core/dirhistory.h"
#include "core/dirfavorite.h"
#include "core/winshell.h"
#include <QDir>
#include <QHBoxLayout>
#include <QToolButton>
#include <QFileIconProvider>
#include <QButtonGroup>
#include <QSettings>
#include <QMenu>

PanelWidget::PanelWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PanelWidget)
    , buttonGroup(new QButtonGroup(this))
    , dirFavorite(new DirFavorite())
    , dirHistory(new DirHistory())
{
    ui->setupUi(this);
    ui->tabWidget->setTabBarAutoHide(true);
    initDrivers();
    connect(buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(buttonClicked(QAbstractButton*)));
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &PanelWidget::currentChanged);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &PanelWidget::tabCloseRequested);
}

PanelWidget::~PanelWidget()
{
    delete dirFavorite;
    delete dirHistory;
    delete ui;
}

void PanelWidget::addDirTab(QWidget* widget, QIcon const& icon, QString const& text)
{
    ui->tabWidget->addTab(widget, icon, text);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
    emit tabCountChanged(ui->tabWidget->count());
}

void PanelWidget::setTabBarAutoHide(int count)
{
    ui->tabWidget->setTabBarAutoHide(count > 1 ? false : true);
}

void PanelWidget::saveSettings(QString const& name)
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup(name);
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

void PanelWidget::loadSettings(QString const& name)
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup(name);
    QStringList dirNames;
    int size = settings.beginReadArray("historyDirNames");
    for(int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        dirNames << settings.value("dirName").toString();
    }
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
    dirHistory->setDirs(dirNames);
}

void PanelWidget::preDir()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(!dir)
        return;
    QString newDir = dirHistory->pre(dir->dir());
    if(!newDir.isEmpty())
        dir->setDir(newDir, QString(), true);
}

void PanelWidget::nextDir()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(!dir)
        return;
    QString newDir = dirHistory->next(dir->dir());
    if(!newDir.isEmpty())
        dir->setDir(newDir, QString(), true);
}

void PanelWidget::addDirToHistory(QString const& dir, bool isNavigation)
{
    if(!isNavigation)
        dirHistory->add(dir);
    updateTexts(ui->tabWidget->currentWidget());
}

void PanelWidget::libDirContextMenu()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(!dir)
        return;

    QMenu menu;

#if 0
    QList<WinLibDir> libDirs = WinShell::winLibDirs();
    foreach(auto const& libDir, libDirs)
    {
        menu.addAction(libDir.icon(), libDir.caption, this, [=](bool){
            dir->setDir(libDir.filePath, libDir.showPath());
        });
    }
#else
    ShellMenuItems menuItems = WinShell::shellMenuItems();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    foreach(auto const& menuItem, menuItems)
    {
        if(menuItem.isDir())
            menu.addAction(menuItem.icon, menuItem.caption, this, [=](bool){
                dir->setDir(menuItem.filePath, menuItem.showPath());
            });
        else
        {
            QMenu* subMenu = new QMenu(menuItem.caption);
            QAction* menuAction = subMenu->menuAction();
            menuAction->setIcon(menuItem.icon);
            connect(menuAction,  &QAction::triggered, this, [=](){
                menuItem.exec();
            });
            connect(menuAction,  &QAction::hovered, this, [=](){
                if(subMenu->isEmpty())
                {
                    ShellMenuItems childMenuItems;
                    WinShell::shellSubMenuItems(menuItem, childMenuItems);
                    if(childMenuItems.isEmpty())
                        menuAction->setMenu(0);
                    foreach(auto const& childMenuItem, childMenuItems)
                    {
                        if(childMenuItem.isDir())
                            subMenu->addAction(childMenuItem.icon, childMenuItem.caption, this, [=](bool){
                                dir->setDir(childMenuItem.filePath, childMenuItem.showPath());
                            });
                       else
                            subMenu->addAction(childMenuItem.icon, childMenuItem.caption, this, [=](bool){
                                childMenuItem.exec();
                            });
                    }
                }
            });
            menu.addAction(menuAction);
        }
    }
#endif
    QApplication::restoreOverrideCursor();
    menu.exec(QCursor::pos());
}

void PanelWidget::favoritesDirContextMenu()
{
    QMenu menu;
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    if(!dir)
        return;

    QString currentFileName = dir->dir();
    QList<FavoriteItem> items = dirFavorite->favoriteItems();
    bool isCurrent = false;
    foreach(auto const& item, items)
    {
        QAction* action = menu.addAction(item.caption, this, [&](bool)
        {
            dir->setDir(item.fileName);
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

void PanelWidget::historyDirContextMenu()
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->currentWidget());
    QStringList const& dirNames = dirHistory->dirs();
    if(!dir || dirNames.isEmpty())
        return;

    QMenu menu;
    QString currentDir = dir->dir();

    foreach(auto const& dirName, dirNames)
    {
        QAction* action = menu.addAction(dirName, this, [&](bool){
            dir->setDir(dirName);
        });

        if(dirName == currentDir)
        {
            action->setCheckable(true);
            action->setChecked(true);
        }
    }
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
                break;
            }
        }
    }
}

void PanelWidget::initDrivers()
{
    QFileInfoList drivers = QDir::drives();
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
        layout->addWidget(button);
        buttonGroup->addButton(button, QChar(ch).unicode());
        button->hide();
    }
    QFileIconProvider fip;
    for(int i = 0; i < drivers.size(); i++)
    {
        QChar ch =drivers[i].path().at(0).toLower();
        QAbstractButton* button = buttonGroup->button(ch.unicode());
        button->setIcon(fip.icon(drivers[i]));
        button->show();
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
    layout->addStretch();

    connect(homeButton, SIGNAL(clicked()), this, SLOT(backToHome()));
    connect(rootButton, SIGNAL(clicked()), this, SLOT(backToRoot()));
    connect(topButton, SIGNAL(clicked()), this, SLOT(backToPrePath()));
    ui->driverWidget->setLayout(layout);
}

void PanelWidget::updateDrivers(bool isAdded)
{
    QFileInfoList drivers = QDir::drives();
    if(isAdded)
    {
        QFileIconProvider fip;
        for(int i = 0; i < drivers.size(); i++)
        {
            QChar ch = drivers[i].path().at(0).toLower();
            QAbstractButton* button = buttonGroup->button(ch.unicode());
            if(button && button->isHidden())
            {
                button->setIcon(fip.icon(drivers[i]));
                button->show();
            }
        }
    }
    else
    {
        QList<int> ids;
        for(int i = 0; i < drivers.size(); i++)
            ids << drivers[i].path().at(0).toLower().unicode();
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
        dir->setDir(dirHistory->find(driver));
}

void PanelWidget::currentChanged(int index)
{
    BaseDir* dir = dynamic_cast<BaseDir *>(ui->tabWidget->widget(index));
    if(!dir)
        return;
     QList<QAbstractButton*> buttons = buttonGroup->buttons();
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
