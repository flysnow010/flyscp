#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "model/localdirmodel.h"
#include "view/panelwidget.h"
#include "view/localdirdockwidget.h"
#include "view/remotedockwidget.h"
#include "view/toolbuttons.h"
#include "dialog/connectdialog.h"
#include "dialog/aboutdialog.h"
#include "util/utils.h"
#include <QSplitter>
#include <QTabWidget>
#include <QSettings>
#include <QDesktopWidget>
#include <QDebug>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , leftPanelWidget(new PanelWidget(this))
    , rightPanelWidget(new PanelWidget(this))
    , leftDirView(new LocalDirDockWidget(this))
    , rightDirView(new LocalDirDockWidget(this))
    , toolButtons(new ToolButtons(this))
{
    ui->setupUi(this);
    QSplitter *spliter = new QSplitter(this);
    spliter->setHandleWidth(0);
    spliter->addWidget(leftPanelWidget);
    spliter->addWidget(rightPanelWidget);

    leftPanelWidget->addDirTab(leftDirView, Utils::driverIcon(), "Local of left");
    rightPanelWidget->addDirTab(rightDirView, Utils::driverIcon(), "Local of right");
    setCentralWidget(spliter);

    QMenuBar *bar = new QMenuBar(ui->menubar);

    QMenu *menuHelp = new QMenu("?", bar);
    bar->addMenu(menuHelp);
    menuHelp->addAction(ui->actionAbout);

    ui->menubar->setCornerWidget(bar);
    ui->statusbar->addWidget(toolButtons, 1);
    load();
    loadStyleSheet();
    createConnects();
}

MainWindow::~MainWindow()
{
    save();
    delete ui;
}

void MainWindow::createConnects()
{
    connect(ui->actionConnect, &QAction::triggered, this, [=](bool){
        ConnectDialog dialog;
        SSHSettings settings;
        //settings.hostName = "192.168.40.80";
        settings.hostName = "13.13.13.159";

        settings.userName = "james";
        dialog.setType(ConnectType::SSH);
        dialog.setSshSettings(settings);
        if(dialog.exec() == QDialog::Accepted)
        {
            RemoteDockWidget* rightDirView = new RemoteDockWidget(this);
            settings = dialog.sshSettings();
            settings.passWord = "james010";
            rightDirView->start(settings);
            rightPanelWidget->addDirTab(rightDirView, Utils::networkIcon(), rightDirView->name());
        }
    });
    connect(ui->actionExit, &QAction::triggered, this, [&](bool) {
        close();
    });

    connect(ui->actionFileFolder, SIGNAL(triggered(bool)), this, SLOT(newFolder()));
    connect(ui->actionViewIt, SIGNAL(triggered(bool)), this, SLOT(viewFile()));
    connect(ui->actionCopy, SIGNAL(triggered(bool)), this, SLOT(copyFiles()));
    connect(ui->actionMove, SIGNAL(triggered(bool)), this, SLOT(moveFiles()));
    connect(ui->actionDelete, SIGNAL(triggered(bool)), this, SLOT(delFiles()));

    connect(ui->actionToolBar,  &QAction::triggered, this, [&](bool on){
        ui->toolBar->setVisible(on);
    });
    connect(ui->actionStatusBar,  &QAction::triggered, this, [&](bool on){
        ui->statusbar->setVisible(on);
    });

    connect(ui->actionAbout,  &QAction::triggered, this, [](bool){
        AboutDialog dialog;
        dialog.exec();
    });
    connect(leftDirView, &LocalDirDockWidget::sectionResized, this, [&](int index, int, int newSize){
        rightDirView->resizeSection(index, newSize);
    });
    connect(rightDirView, &LocalDirDockWidget::sectionResized, this, [&](int index, int, int newSize){
        leftDirView->resizeSection(index, newSize);
    });
    connect(leftPanelWidget, &PanelWidget::tabCountChanged, this, [&](int count){
        rightPanelWidget->setTabBarAutoHide(count);
    });
    connect(rightPanelWidget, &PanelWidget::tabCountChanged, this, [&](int count){
        leftPanelWidget->setTabBarAutoHide(count);
    });
    connect(leftDirView, &LocalDirDockWidget::dirChanged, this, [&](QString const& dir, bool isRemote){
        leftPanelWidget->addDirToHistory(dir, isRemote);
    });
    connect(rightDirView, &LocalDirDockWidget::dirChanged, this, [&](QString const& dir, bool isRemote){
        rightPanelWidget->addDirToHistory(dir, isRemote);
    });

    connect(leftDirView, &LocalDirDockWidget::libDirContextMenuRequested, this, [&](){
        leftPanelWidget->libDirContextMenu();
    });
    connect(leftDirView, &LocalDirDockWidget::favoritesDirContextMenuRequested, this, [&](){
        leftPanelWidget->favoritesDirContextMenu();
    });
    connect(leftDirView, &LocalDirDockWidget::historyDirContextMenuRequested, this, [&](){
        leftPanelWidget->historyDirContextMenu();
    });

    connect(rightDirView, &LocalDirDockWidget::libDirContextMenuRequested, this, [&](){
        rightPanelWidget->libDirContextMenu();
    });
    connect(rightDirView, &LocalDirDockWidget::favoritesDirContextMenuRequested, this, [&](){
        rightPanelWidget->favoritesDirContextMenu();
    });
    connect(rightDirView, &LocalDirDockWidget::historyDirContextMenuRequested, this, [&](){
        rightPanelWidget->historyDirContextMenu();
    });

    connect(rightDirView, &LocalDirDockWidget::actived, this, [&](){
        leftDirView->setActived(false);
    });
    connect(leftDirView, &LocalDirDockWidget::actived, this, [&](){
        rightDirView->setActived(false);
    });
    connect(toolButtons, &ToolButtons::newClicked, this, &MainWindow::newFolder);
    connect(toolButtons, &ToolButtons::viewClicked, this, &MainWindow::viewFile);
    connect(toolButtons, &ToolButtons::editClicked, this, &MainWindow::editFile);
    connect(toolButtons, &ToolButtons::copyClicked, this, &MainWindow::copyFiles);
    connect(toolButtons, &ToolButtons::moveClicked, this, &MainWindow::moveFiles);
    connect(toolButtons, &ToolButtons::deleteClicked, this, &MainWindow::delFiles);
    connect(toolButtons, &ToolButtons::exitClicked, this, [=](){ close(); });
}

void MainWindow::save()
{
    saveSettings();
    leftDirView->saveSettings("LeftDirView");
    rightDirView->saveSettings("RightDirView");
    leftPanelWidget->saveSettings("LeftPanel");
    rightPanelWidget->saveSettings("RightPane");
}

void MainWindow::load()
{
    loadSettings();
    leftDirView->loadSettings("LeftDirView");
    rightDirView->loadSettings("RightDirView");
    leftPanelWidget->loadSettings("LeftPanel");
    rightPanelWidget->loadSettings("RightPane");
    leftPanelWidget->updateTexts(leftDirView);
    rightPanelWidget->updateTexts(rightDirView);
    leftDirView->setActived(true);
}

void MainWindow::saveSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("toolBarIsVisible", ui->actionToolBar->isChecked());
    settings.setValue("statusBarIsVisible", ui->actionStatusBar->isChecked());
}

void MainWindow::loadSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    const QByteArray windowState = settings.value("windowState", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
    if(!windowState.isEmpty())
        restoreState(windowState);
    bool toolBarIsVisible = settings.value("toolBarIsVisible", true).toBool();
    bool statusBarIsVisible = settings.value("statusBarIsVisible", true).toBool();
    ui->actionToolBar->setChecked(toolBarIsVisible);
    ui->actionStatusBar->setChecked(statusBarIsVisible);
    ui->toolBar->setVisible(toolBarIsVisible);
    ui->statusbar->setVisible(statusBarIsVisible);
}

void MainWindow::loadStyleSheet()
{
    QString fileName = QString("%1/skins/main.css").arg(Utils::currentPath());
    setStyleSheet(QString::fromUtf8(Utils::readFile(fileName)));
}

void MainWindow::newFolder()
{
    if(leftDirView->isActived())
        leftDirView->newFolder();
    else
        rightDirView->newFolder();
}

void MainWindow::viewFile()
{
    if(leftDirView->isActived())
        leftDirView->viewFile();
    else
        rightDirView->viewFile();
}

void MainWindow::editFile()
{
    if(leftDirView->isActived())
        leftDirView->viewFile();
    else
        rightDirView->viewFile();
}

void MainWindow::copyFiles()
{
    if(leftDirView->isActived())
    {
        leftDirView->copyFiles(rightDirView->dir());
        rightDirView->refresh();
    }
    else
    {
        rightDirView->copyFiles(leftDirView->dir());
        leftDirView->refresh();
    }
}

void MainWindow::moveFiles()
{
    if(leftDirView->isActived())
    {
        leftDirView->moveFiles(rightDirView->dir());
        rightDirView->refresh();
    }
    else
    {
        rightDirView->moveFiles(leftDirView->dir());
        leftDirView->refresh();
    }
}

void MainWindow::delFiles()
{
    if(leftDirView->isActived())
        leftDirView->delFilesWithConfirm();
    else
        rightDirView->delFilesWithConfirm();
}

void MainWindow::selectAll()
{
    if(leftDirView->isActived())
        leftDirView->selectAll();
    else
        rightDirView->selectAll();
}
