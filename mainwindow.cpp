#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "model/localdirmodel.h"
#include "view/panelwidget.h"
#include "view/localdirdockwidget.h"
#include "view/remotedockwidget.h"
#include "view/statusbar.h"
#include "view/commandbar.h"
#include "view/toolbuttons.h"
#include "dialog/connectdialog.h"
#include "dialog/optionsdialog.h"
#include "dialog/networksettingsdialog.h"
#include "dialog/aboutdialog.h"
#include "util/utils.h"
#include "core/winshell.h"
#include "core/sshsettings.h"
#include <QSplitter>
#include <QTabWidget>
#include <QSettings>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <QScreen>

#include <windows.h>
#include <dbt.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sshSettingsMangaer_(new SSHSettingsManager)
    , connectMenu(new QMenu(this))
    , diffMenu(new QMenu(this))
    , leftPanelWidget(new PanelWidget(this))
    , rightPanelWidget(new PanelWidget(this))
    , leftDirView(new LocalDirDockWidget(this))
    , rightDirView(new LocalDirDockWidget(this))
    , toolButtons(new ToolButtons(this))
{
    ui->setupUi(this);
    QWidget* centerWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout();
    QSplitter *spliter = new QSplitter(this);
    statusBar = new StatusBar(this);
    commandBar = new CommandBar(this);
    spliter->setHandleWidth(0);
    spliter->addWidget(leftPanelWidget);
    spliter->addWidget(rightPanelWidget);
    layout->addWidget(spliter);
    layout->addWidget(statusBar);
    layout->addWidget(commandBar);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setStretch(0, 1);
    layout->setStretch(1, 0);
    layout->setStretch(2, 0);
    centerWidget->setLayout(layout);
    connect(spliter, &QSplitter::splitterMoved, this, [=](int pos, int){
       statusBar->setLeftWidth(pos);
    });

    leftPanelWidget->addDirTab(leftDirView, Utils::driverIcon(), "Local of left");
    rightPanelWidget->addDirTab(rightDirView, Utils::driverIcon(), "Local of right");
    setCentralWidget(centerWidget);

    createHelpMenu();
    createDiffMenu();
    createToolButtons();
    createConnects();
    load();
    updateConnectMenu();
    loadStyleSheet();
}

MainWindow::~MainWindow()
{
    save();
    delete sshSettingsMangaer_;
    delete ui;
}

void MainWindow::createHelpMenu()
{
    QMenuBar *bar = new QMenuBar(ui->menubar);
    QMenu *menuHelp = new QMenu("?", bar);
    bar->addMenu(menuHelp);
    menuHelp->addAction(ui->actionAbout);
    ui->menubar->setCornerWidget(bar);

    ui->buttonsBar->addWidget(toolButtons, 1);
}

void MainWindow::createDiffMenu()
{
    diffMenu->addAction("Diff files", this, [=](){
        diffFiles();
    });
}

void MainWindow::createToolButtons()
{
    diffMenu->setIcon(QIcon(":/image/diff.png"));
    connectMenu->setIcon(QIcon(":/image/connect.png"));
    QAction* diffAction = diffMenu->menuAction();
    QAction* connectAction = connectMenu->menuAction();
    diffAction->setStatusTip("Diff tow folders");
    connectAction->setStatusTip("Connect a stfp");

    connect(diffAction, SIGNAL(triggered(bool)), this, SLOT(diffFolders()));
    connect(connectAction, SIGNAL(triggered(bool)), this, SLOT(connectSftp()));
    ui->toolBar->addAction(diffAction);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionControlPanel);
    ui->toolBar->addAction(ui->actionCalc);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(connectAction);
}

void MainWindow::updateConnectMenu()
{
    connectMenu->clear();
    for(int i = 0; i < sshSettingsMangaer_->size(); i++)
    {
        SSHSettings::Ptr settings = sshSettingsMangaer_->settings(i);
        connectMenu->addAction(settings->name, this, [=](){
                settings->passWord = Utils::getPassword(QString("Input Password for %1").arg(settings->name));
                if(settings->passWord.isEmpty())
                    return;
                createRemoteDirWidget(*settings);
        });
    }
    if(sshSettingsMangaer_->size() > 0)
        connectMenu->addSeparator();
    connectMenu->addAction("Settings", this, [=](){
        NetworkSettingsDialog dialog(this);
        dialog.exec();
    });
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType)
    MSG* msg = (MSG*)message;
    if(msg->message == WM_DEVICECHANGE)
    {
        if(msg->wParam == DBT_DEVICEARRIVAL)
        {
            PDEV_BROADCAST_HDR devHdr = (PDEV_BROADCAST_HDR)msg->lParam;
            if(devHdr->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                leftPanelWidget->updateDrivers(true);
                rightPanelWidget->updateDrivers(true);
            }
            *result = 0;
        }
        else if(msg->wParam == DBT_DEVICEREMOVECOMPLETE)
        {
            PDEV_BROADCAST_HDR devHdr = (PDEV_BROADCAST_HDR)msg->lParam;
            if(devHdr->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                leftPanelWidget->updateDrivers(false);
                rightPanelWidget->updateDrivers(false);
            }
            *result = 0;
        }
    }
    return false;
}

void MainWindow::createConnects()
{
    createMenuConnect();
    createViewConnect();
    createButtonsConnect();
}

void MainWindow::createMenuConnect()
{
    connect(ui->actionConnect, SIGNAL(triggered(bool)), this, SLOT(connectSftp()));
    connect(ui->actionExit, &QAction::triggered, this, [&](bool) { close(); });
    connect(ui->actionFileFolder, SIGNAL(triggered(bool)), this, SLOT(newFolder()));
    connect(ui->actionTextFile, SIGNAL(triggered(bool)), this, SLOT(newFile()));
    connect(ui->actionViewIt, SIGNAL(triggered(bool)), this, SLOT(viewFile()));
    connect(ui->actionCopy, SIGNAL(triggered(bool)), this, SLOT(copyFiles()));
    connect(ui->actionMove, SIGNAL(triggered(bool)), this, SLOT(moveFiles()));
    connect(ui->actionDelete, SIGNAL(triggered(bool)), this, SLOT(delFiles()));
    connect(ui->actionSelectAll, SIGNAL(triggered(bool)), this, SLOT(selectAll()));
    connect(ui->actionCompress, SIGNAL(triggered(bool)), this, SLOT(compressFiles()));
    connect(ui->actionUncompress, SIGNAL(triggered(bool)), this, SLOT(uncompressFiles()));
    connect(ui->actionSearch, SIGNAL(triggered(bool)), this, SLOT(searchFiles()));
    connect(ui->actionControlPanel, &QAction::triggered, this, [&](){
        WinShell::Exec("control");
    });
    connect(ui->actionCalc, &QAction::triggered, this, [&](){
        WinShell::Exec("calc");
    });
    connect(ui->actionSettings, &QAction::triggered, this, [&](){
        NetworkSettingsDialog dialog;
        if(dialog.exec() == QDialog::Accepted)
        {
            ;
        }
    });
    connect(ui->actionOption, &QAction::triggered, this, [&](){
        OptionsDialog dialog;
        if(dialog.exec() == QDialog::Accepted)
        {
            ;
        }
    });
    connect(ui->actionRefresh, &QAction::triggered, this, [&](){
        if(leftDirView->isActived())
            leftDirView->refresh();
        else
            rightDirView->refresh();
    });
    connect(ui->actionPrevious, &QAction::triggered, this, [&](){
        if(leftDirView->isActived())
            leftPanelWidget->preDir();
        else
            rightPanelWidget->preDir();
    });
    connect(ui->actionNext, &QAction::triggered, this, [&](){
        if(leftDirView->isActived())
            leftPanelWidget->nextDir();
        else
            rightPanelWidget->nextDir();
    });


    connect(ui->actionToolBar,  &QAction::triggered, this, [&](bool on){
        ui->toolBar->setVisible(on);
    });
    connect(ui->actionStatusBar,  &QAction::triggered, this, [&](bool on){
        statusBar->setVisible(on);
    });
    connect(ui->actionCommandBar,  &QAction::triggered, this, [&](bool on){
        commandBar->setVisible(on);
    });
    connect(ui->actionButtonsBar,  &QAction::triggered, this, [&](bool on){
        ui->buttonsBar->setVisible(on);
    });

    connect(ui->actionAbout,  &QAction::triggered, this, [](bool){ AboutDialog().exec(); });
}

void MainWindow::createViewConnect()
{
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
    connect(leftDirView, &LocalDirDockWidget::dirChanged, this, [&](QString const& dir, bool isNavigation){
        leftPanelWidget->addDirToHistory(dir, isNavigation);
    });
    connect(rightDirView, &LocalDirDockWidget::dirChanged, this, [&](QString const& dir, bool isNavigation){
        rightPanelWidget->addDirToHistory(dir, isNavigation);
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
    connect(leftDirView, &LocalDirDockWidget::statusTextChanged, this, [&](QString const& text){
        statusBar->setLeftStatusText(text);
    });
    connect(rightDirView, &LocalDirDockWidget::statusTextChanged, this, [&](QString const& text){
        statusBar->setRightStatusText(text);
    });
    connect(leftDirView, &LocalDirDockWidget::dirChanged, this, [&](QString const& dir, bool){
        commandBar->setDir(dir);
    });
    connect(commandBar, &CommandBar::commanded, this, [&](QString const& commnad){
        if(leftDirView->isActived())
            leftDirView->execCommand(commnad);
        else
            rightDirView->execCommand(commnad);
    });
}

void MainWindow::createButtonsConnect()
{
    connect(toolButtons, &ToolButtons::newFolderClicked, this, &MainWindow::newFolder);
    connect(toolButtons, &ToolButtons::newFileClicked, this, &MainWindow::newFile);
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
    sshSettingsMangaer_->save(QString("%1/settings.json").arg(Utils::sshSettingsPath()));
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
    sshSettingsMangaer_->load(QString("%1/settings.json").arg(Utils::sshSettingsPath()));
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
    settings.setValue("commandBarIsVisible", ui->actionCommandBar->isChecked());
    settings.setValue("ButtonsBarIsVisible", ui->actionButtonsBar->isChecked());
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
    bool commandBarIsVisible = settings.value("commandBarIsVisible", true).toBool();
    bool buttonsBarIsVisible = settings.value("ButtonsBarIsVisible", true).toBool();
    ui->actionToolBar->setChecked(toolBarIsVisible);
    ui->actionStatusBar->setChecked(statusBarIsVisible);
    ui->actionCommandBar->setChecked(commandBarIsVisible);
    ui->actionButtonsBar->setChecked(buttonsBarIsVisible);
    ui->toolBar->setVisible(toolBarIsVisible);
    statusBar->setVisible(statusBarIsVisible);
    commandBar->setVisible(commandBarIsVisible);
    ui->buttonsBar->setVisible(buttonsBarIsVisible);
}

void MainWindow::loadStyleSheet()
{
    setStyleSheet("QTabBar::close-button {"
                  "image: url(:/image/close.png);}"
                  "QTabBar::close-button:hover {"
                  "image: url(:/image/close-hover.png);}");
}

void MainWindow::newFolder()
{
    if(leftDirView->isActived())
        leftDirView->newFolder();
    else
        rightDirView->newFolder();
}

void MainWindow::newFile()
{
    if(leftDirView->isActived())
        leftDirView->newTxtFile();
    else
        rightDirView->newTxtFile();
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

void MainWindow::compressFiles()
{
    if(leftDirView->isActived())
    {
        leftDirView->compressFiles(rightDirView->dir());
        rightDirView->refresh();
    }
    else
    {
        rightDirView->compressFiles(leftDirView->dir());
        leftDirView->refresh();
    }
}

void MainWindow::uncompressFiles()
{
    if(leftDirView->isActived())
    {
        leftDirView->uncompressFiles(rightDirView->dir());
        rightDirView->refresh();
    }
    else
    {
        rightDirView->uncompressFiles(leftDirView->dir());
        leftDirView->refresh();
    }
}

void MainWindow::searchFiles()
{
    if(leftDirView->isActived())
        leftDirView->searchFiles(leftDirView->dir());
    else
        rightDirView->searchFiles(rightDirView->dir());
}

void MainWindow::diffFolders()
{
    QStringList params;
    params << "-contextdiff"
           << leftDirView->dir() << rightDirView->dir();
    FileNames::MakeFileNamesAsParams(params);
    WinShell::Exec(Utils::diffApp(), params);
}

void MainWindow::diffFiles()
{
    QStringList params;
    params << "-contextdiff"
           << leftDirView->currentFileName() << rightDirView->currentFileName();
    FileNames::MakeFileNamesAsParams(params);
    WinShell::Exec(Utils::diffApp(), params);
}

void MainWindow::connectSftp()
{
    ConnectDialog dialog;

    dialog.setType(ConnectType::SSH);
    if(dialog.exec() == QDialog::Accepted)
    {
        SSHSettings::Ptr settings = dialog.sshSettings();
        sshSettingsMangaer_->addSettings(settings);
        updateConnectMenu();
        settings->passWord = Utils::getPassword(QString("Input Password for %1").arg(settings->name));
        if(settings->passWord.isEmpty())
            return;
        createRemoteDirWidget(*settings);
    }
}

void MainWindow::createRemoteDirWidget(SSHSettings const& settings)
{
    RemoteDockWidget* remoteDockWidget = new RemoteDockWidget(this);
    remoteDockWidget->start(settings);

    if(rightPanelWidget->tabCount() <= leftPanelWidget->tabCount())
    {
        rightPanelWidget->addDirTab(remoteDockWidget, Utils::networkIcon(), remoteDockWidget->name());
        connect(remoteDockWidget, &RemoteDockWidget::closeRequest, rightPanelWidget, &PanelWidget::closeTab);
        connect(leftDirView, &LocalDirDockWidget::remoteDownload, this, [=](
                QString const& remoteSrc, QStringList const& fileNames,
                QString const& targetFilePath) {
            remoteDockWidget->downloadFiles(remoteSrc, fileNames, targetFilePath);
            leftDirView->refresh();
        });
    }
    else
    {
        leftPanelWidget->addDirTab(remoteDockWidget, Utils::networkIcon(), remoteDockWidget->name());
        connect(remoteDockWidget, &RemoteDockWidget::closeRequest, leftPanelWidget, &PanelWidget::closeTab);
        connect(rightDirView, &LocalDirDockWidget::remoteDownload, this, [=](
                QString const& remoteSrc, QStringList const& fileNames,
                QString const& targetFilePath) {
            remoteDockWidget->downloadFiles(remoteSrc, fileNames, targetFilePath);
            rightDirView->refresh();
        });
    }
}
