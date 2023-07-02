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
#include "core/languagemanager.h"

#include <QSplitter>
#include <QTabWidget>
#include <QSettings>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <QScreen>
#include <QStyleFactory>


#include <windows.h>
#include <dbt.h>

QTranslator MainWindow::appTranslator;
QTranslator MainWindow::sysTranslator;

void MainWindow::InstallTranstoirs(bool isInited)
{
    if(!isInited)
        theOptionManager.load("Options");

    Language lang;
    LanguageManager().find(theOptionManager.languageOption().language, lang);

    if(isInited)
    {
        qApp->removeTranslator(&appTranslator);
        qApp->removeTranslator(&sysTranslator);
    }

    if(!lang.isDefault())
    {
        appTranslator.load(QString("%1/%2.qm").arg(Utils::languagePath(), lang.file));
        sysTranslator.load(QString("%1/qt_%2.qm").arg(Utils::languagePath(), lang.file));
        qApp->installTranslator(&appTranslator);
        qApp->installTranslator(&sysTranslator);
    }
}

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
    , isShowTips_(true)
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

    leftPanelWidget->addDirTab(leftDirView, Utils::driverIcon(), tr("Local of left"));
    rightPanelWidget->addDirTab(rightDirView, Utils::driverIcon(), tr("Local of right"));
    setCentralWidget(centerWidget);

    createHelpMenu();
    createDiffMenu();
    createToolButtons();
    createConnects();
    load();
    updateConnectMenu();
    loadStyleSheet();
    installEventFilter(this);
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
    diffAction->setToolTip(tr("Diff folders"));
    connectAction->setToolTip(tr("Connect"));
    diffAction->setStatusTip(tr("Diff two folders"));
    connectAction->setStatusTip(tr("Connect a stfp"));

    connect(diffAction, SIGNAL(triggered(bool)),
            this, SLOT(diffFolders()));
    connect(connectAction, SIGNAL(triggered(bool)),
            this, SLOT(connectSftp()));
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
                settings->passWord = Utils::getPassword(QString(tr("Password for %1"))
                                                        .arg(settings->name));
                if(settings->passWord.isEmpty())
                    return;
                createRemoteDirWidget(*settings);
        });
    }
    if(sshSettingsMangaer_->size() > 0)
        connectMenu->addSeparator();
    connectMenu->addAction(tr("Settings"), this, SLOT(netsettings()));
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

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{

    if(obj->objectName() == "toolBar")
    {
        qDebug() << event->type();
    }
    if(event->type() == QEvent::ToolTip || event->type() == QEvent::StatusTip)
        return !isShowTips_;
    return QMainWindow::eventFilter(obj, event);
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
    connect(ui->actionOption, SIGNAL(triggered(bool)), this, SLOT(options()));
    connect(ui->actionSettings, SIGNAL(triggered(bool)), this, SLOT(netsettings()));

    connect(ui->actionControlPanel, &QAction::triggered, this, [&](){
        WinShell::Exec("control");
    });

    connect(ui->actionCalc, &QAction::triggered, this, [&](){
        WinShell::Exec("calc");
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
        LayoutOption option = theOptionManager.layoutOption();
        option.isShowToolBar = on;
        ui->toolBar->setVisible(on);
        theOptionManager.setLayoutOption(option);
    });

    connect(ui->actionStatusBar,  &QAction::triggered, this, [&](bool on){
        LayoutOption option = theOptionManager.layoutOption();
        option.isShowStatusBar = on;
        statusBar->setVisible(on);
        theOptionManager.setLayoutOption(option);
    });

    connect(ui->actionCommandBar,  &QAction::triggered, this, [&](bool on){
        LayoutOption option = theOptionManager.layoutOption();
        option.isShowCommandLine = on;
        commandBar->setVisible(on);
        theOptionManager.setLayoutOption(option);
    });

    connect(ui->actionButtonsBar,  &QAction::triggered, this, [&](bool on){
        LayoutOption option = theOptionManager.layoutOption();
        option.isShowFunctionKeyButtons = on;
        ui->buttonsBar->setVisible(on);
        theOptionManager.setLayoutOption(option);
    });

    connect(ui->actionAbout,  &QAction::triggered,
            this, [](bool){ AboutDialog().exec(); });
}

void MainWindow::createViewConnect()
{
    connect(leftDirView, &LocalDirDockWidget::sectionResized,
            this, [&](int index, int, int newSize){
        rightDirView->resizeSection(index, newSize);
    });

    connect(rightDirView, &LocalDirDockWidget::sectionResized,
            this, [&](int index, int, int newSize){
        leftDirView->resizeSection(index, newSize);
    });

    connect(leftPanelWidget, &PanelWidget::tabCountChanged,
            this, [&](int count){
        rightPanelWidget->setTabBarAutoHide(count);
    });

    connect(rightPanelWidget, &PanelWidget::tabCountChanged,
            this, [&](int count){
        leftPanelWidget->setTabBarAutoHide(count);
    });

    connect(leftDirView, &LocalDirDockWidget::dirChanged,
            this, [&](QString const& dir, bool isNavigation){
        leftPanelWidget->addDirToHistory(dir, isNavigation);
    });

    connect(rightDirView, &LocalDirDockWidget::dirChanged,
            this, [&](QString const& dir, bool isNavigation){
        rightPanelWidget->addDirToHistory(dir, isNavigation);
    });

    connect(leftDirView, &LocalDirDockWidget::libDirContextMenuRequested,
            this, [&]()
    {
        leftPanelWidget->libDirContextMenu();
    });

    connect(leftDirView, &LocalDirDockWidget::favoritesDirContextMenuRequested,
            this, [&]()
    {
        leftPanelWidget->favoritesDirContextMenu();
    });

    connect(leftDirView, &LocalDirDockWidget::historyDirContextMenuRequested,
            this, [&]()
    {
        leftPanelWidget->historyDirContextMenu();
    });

    connect(leftDirView, &LocalDirDockWidget::copyRequested,
            this, [&]()
    {
        leftDirView->copyFiles(rightDirView->dir());
    });

    connect(leftDirView, &LocalDirDockWidget::moveRequested,
            this, [&]()
    {
        leftDirView->moveFiles(rightDirView->dir());
    });

    connect(rightDirView, &LocalDirDockWidget::libDirContextMenuRequested,
            this, [&]()
    {
        rightPanelWidget->libDirContextMenu();
    });

    connect(rightDirView, &LocalDirDockWidget::favoritesDirContextMenuRequested,
            this, [&]()
    {
        rightPanelWidget->favoritesDirContextMenu();
    });

    connect(rightDirView, &LocalDirDockWidget::historyDirContextMenuRequested,
            this, [&]()
    {
        rightPanelWidget->historyDirContextMenu();
    });

    connect(rightDirView, &LocalDirDockWidget::copyRequested,
            this, [&]()
    {
        rightDirView->copyFiles(leftDirView->dir());
    });

    connect(rightDirView, &LocalDirDockWidget::moveRequested,
            this, [&]()
    {
        rightDirView->moveFiles(leftDirView->dir());
    });

    connect(rightDirView, &LocalDirDockWidget::actived,
            this, [&]()
    {
        leftDirView->setActived(false);
        commandBar->setDir(rightDirView->dir());
    });
    connect(leftDirView, &LocalDirDockWidget::actived, this, [&](){
        rightDirView->setActived(false);
        commandBar->setDir(leftDirView->dir());
    });

    connect(leftDirView, &LocalDirDockWidget::statusTextChanged,
            this, [&](QString const& text)
    {
        statusBar->setLeftStatusText(text);
    });
    connect(rightDirView, &LocalDirDockWidget::statusTextChanged,
            this, [&](QString const& text)
    {
        statusBar->setRightStatusText(text);
    });

    connect(leftDirView, &LocalDirDockWidget::dirChanged,
            this, [&](QString const& dir, bool)
    {
        commandBar->setDir(dir);
    });

    connect(rightDirView, &LocalDirDockWidget::dirChanged,
            this, [&](QString const& dir, bool)
    {
        commandBar->setDir(dir);
    });

    connect(leftDirView, &LocalDirDockWidget::compressFileExtract,
            this, [&](QStringList const& fileNames)
    {
        rightDirView->extractFiles(fileNames, leftDirView->dir());
        rightDirView->refresh();
    });

    connect(rightDirView, &LocalDirDockWidget::compressFileExtract,
            this, [&](QStringList const& fileNames)
    {
        leftDirView->extractFiles(fileNames, rightDirView->dir());
        leftDirView->refresh();
    });

    connect(commandBar, &CommandBar::commanded,
            this, [&](QString const& commnad)
    {
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
    sshSettingsMangaer_->save(QString("%1/settings.json")
                              .arg(Utils::sshSettingsPath()));
    theOptionManager.save("Options");
}

void MainWindow::load()
{
    bool leftDirViewIsActived = loadSettings();
    leftDirView->loadSettings("LeftDirView");
    rightDirView->loadSettings("RightDirView");
    leftPanelWidget->loadSettings("LeftPanel");
    rightPanelWidget->loadSettings("RightPane");
    leftPanelWidget->updateTexts(leftDirView);
    rightPanelWidget->updateTexts(rightDirView);
    sshSettingsMangaer_->load(QString("%1/settings.json")
                              .arg(Utils::sshSettingsPath()));
    if(leftDirViewIsActived)
    {
        emit leftDirView->actived();
        leftDirView->setActived(true);
    }
    else
    {
        emit rightDirView->actived();
        rightDirView->setActived(true);
    }
    theOptionManager.load("Options");
    updateLayout(theOptionManager.layoutOption());
    updateDisplay(theOptionManager.displayOption());
    updateIcons(theOptionManager.iconOption());
    updateFonts(theOptionManager.fontOption());
    updateColors(theOptionManager.colorOption());
    updateOperation(theOptionManager.operationOption());

    leftPanelWidget->refresh();
    rightPanelWidget->refresh();
}

void MainWindow::saveSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("LeftDirViewIsActived", leftDirView->isActived());
}

bool MainWindow::loadSettings()
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

    return settings.value("LeftDirViewIsActived", true).toBool();
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
        settings->passWord = Utils::getPassword(QString(tr("Password for %1"))
                                                .arg(settings->name));
        if(settings->passWord.isEmpty())
            return;
        createRemoteDirWidget(*settings);
    }
}

void MainWindow::options()
{
    OptionsDialog dialog;
    connect(&dialog, &OptionsDialog::layoutOptionChanged,
            this, [=](LayoutOption const& option)
    {
        updateLayout(option);
        theOptionManager.setLayoutOption(option);
    });

    connect(&dialog, &OptionsDialog::displayOptionChanged,
            this, [=](DisplayOption const& option)
    {
        updateDisplay(option);
        theOptionManager.setDialayOption(option);
    });

    connect(&dialog, &OptionsDialog::iconsOptionChanged,
            this, [=](IconsOption const& option)
    {
        updateIcons(option);
        theOptionManager.setIconsOption(option);
    });

    connect(&dialog, &OptionsDialog::fontOptionChanged,
            this, [=](FontOption const& option)
    {
        updateFonts(option);
        theOptionManager.setFontOption(option);
    });

    connect(&dialog, &OptionsDialog::colorOptionChanged,
            this, [=](ColorOption const& option)
    {
        updateColors(option);
        theOptionManager.setColorOption(option);
    });

    connect(&dialog, &OptionsDialog::langOptionChanged,
            this, [=](LanguageOption const& option)
    {
        theOptionManager.setLanguageOption(option);
        InstallTranstoirs(true);

        ui->retranslateUi(this);
        toolButtons->retranslateUi();
        leftPanelWidget->retranslateUi();
        rightPanelWidget->retranslateUi();
    });

    connect(&dialog, &OptionsDialog::operationOptionChanged,
            this, [=](OperationOption const& option)
    {
        updateOperation(option);
        theOptionManager.setOperationOption(option);
    });

    if(dialog.exec() == QDialog::Accepted)
        dialog.updateUIToOption();
}

void MainWindow::netsettings()
{
    NetworkSettingsDialog dialog;
    dialog.setManager(sshSettingsMangaer_);
    if(dialog.exec() == QDialog::Accepted)
    {
        int index = dialog.connectIndex();
        SSHSettings::Ptr settings = sshSettingsMangaer_->settings(index);
        if(settings)
        {
            settings->passWord = Utils::getPassword(QString(tr("Password for %1"))
                                                    .arg(settings->name));
            if(settings->passWord.isEmpty())
                return;
            createRemoteDirWidget(*settings);
        }
    }
    updateConnectMenu();
}

void MainWindow::createRemoteDirWidget(SSHSettings const& settings)
{
    RemoteDockWidget* remoteDockWidget = new RemoteDockWidget(this);
    remoteDockWidget->start(settings);

    if(rightPanelWidget->tabCount() <= leftPanelWidget->tabCount())
    {
        rightPanelWidget->addDirTab(remoteDockWidget,
                                    Utils::networkIcon(),
                                    remoteDockWidget->name());
        connect(remoteDockWidget, &RemoteDockWidget::closeRequest,
                rightPanelWidget, &PanelWidget::closeTab);

        connect(remoteDockWidget, &RemoteDockWidget::statusTextChanged,
                this, [&](QString const& text)
        {
            statusBar->setRightStatusText(text);
        });

        connect(leftDirView, &LocalDirDockWidget::remoteDownload, this, [=](
                QString const& remoteSrc, QStringList const& fileNames,
                QString const& targetFilePath)
        {
            remoteDockWidget->downloadFiles(remoteSrc, fileNames, targetFilePath);
            leftDirView->refresh();
        });
    }
    else
    {
        leftPanelWidget->addDirTab(remoteDockWidget,
                                   Utils::networkIcon(),
                                   remoteDockWidget->name());
        connect(remoteDockWidget, &RemoteDockWidget::closeRequest,
                leftPanelWidget, &PanelWidget::closeTab);

        connect(remoteDockWidget, &RemoteDockWidget::statusTextChanged,
                this, [&](QString const& text)
        {
            statusBar->setLeftStatusText(text);
        });

        connect(rightDirView, &LocalDirDockWidget::remoteDownload, this, [=](
                QString const& remoteSrc, QStringList const& fileNames,
                QString const& targetFilePath)
        {
            remoteDockWidget->downloadFiles(remoteSrc, fileNames, targetFilePath);
            rightDirView->refresh();
        });
    }
}

void MainWindow::updateLayout(LayoutOption const& o)
{
    ui->toolBar->setVisible(o.isShowToolBar);
    ui->buttonsBar->setVisible(o.isShowFunctionKeyButtons);
    statusBar->setVisible(o.isShowStatusBar);
    commandBar->setVisible(o.isShowCommandLine);

    ui->actionToolBar->setChecked(o.isShowToolBar);
    ui->actionButtonsBar->setChecked(o.isShowFunctionKeyButtons);
    ui->actionStatusBar->setChecked(o.isShowStatusBar);
    ui->actionCommandBar->setChecked(o.isShowCommandLine);

    leftPanelWidget->showDriveButtons(o.isShowDriveButtons);
    leftPanelWidget->showHeader(o.isShowSortHeader);
    leftPanelWidget->showCurrentDir(o.isShowCurrentDir);
    leftPanelWidget->showDeskNavigationButton(o.isShowDeskNavigationButton);
    leftPanelWidget->showFavoriteButton(o.isShowFavoriteButton);
    leftPanelWidget->showHistoryButton(o.isShowHistoryButton);
    rightPanelWidget->showDriveButtons(o.isShowDriveButtons);
    rightPanelWidget->showHeader(o.isShowSortHeader);
    rightPanelWidget->showCurrentDir(o.isShowCurrentDir);
    rightPanelWidget->showDeskNavigationButton(o.isShowDeskNavigationButton);
    rightPanelWidget->showFavoriteButton(o.isShowFavoriteButton);
    rightPanelWidget->showHistoryButton(o.isShowHistoryButton);

    QApplication::setStyle(QStyleFactory::create(o.showStyle));

}

void MainWindow::updateDisplay(DisplayOption const& o, bool isRefresh)
{
    leftPanelWidget->showHiddenAndSystem(o.isShowHideAndSystemFile);
    leftPanelWidget->showParentInRoot(o.isShowParentDirInRootDrive);
    leftPanelWidget->showDriveToolTips(o.isShowDriveTooltips);
    leftPanelWidget->showToolTips(o.isShowFilenameTooltips);
    leftPanelWidget->setDirSoryByTime(!o.isDirSortByName);
    isShowTips_ = o.isShowToolbarTooltips;

    rightPanelWidget->showHiddenAndSystem(o.isShowHideAndSystemFile);
    rightPanelWidget->showParentInRoot(o.isShowParentDirInRootDrive);
    rightPanelWidget->showDriveToolTips(o.isShowDriveTooltips);
    rightPanelWidget->showToolTips(o.isShowFilenameTooltips);
    rightPanelWidget->setDirSoryByTime(!o.isDirSortByName);

    if(isRefresh)
    {
        leftPanelWidget->refresh();
        rightPanelWidget->refresh();
    }
}

void MainWindow::updateIcons(IconsOption const& o, bool isRefresh)
{
    leftPanelWidget->showAllIconWithExeAndLink(o.isShowAllIconIncludeExeAndLink);
    leftPanelWidget->showAllIcon(o.isShowAllIcon);
    leftPanelWidget->showStandardIcon(o.isShowStandardIcon);
    leftPanelWidget->showNoneIcon(o.isNoShowIcon);
    leftPanelWidget->showIconForFyleSystem(o.isShowIconForFilesystem);
    leftPanelWidget->showIconForVirtualFolder(o.isShowOverlayIcon);
    leftPanelWidget->fileIconSize(o.fileIconSize);

    ui->toolBar->setIconSize(QSize(o.toolbarIconSize, o.toolbarIconSize));

    rightPanelWidget->showAllIconWithExeAndLink(o.isShowAllIconIncludeExeAndLink);
    rightPanelWidget->showAllIcon(o.isShowAllIcon);
    rightPanelWidget->showStandardIcon(o.isShowStandardIcon);
    rightPanelWidget->showNoneIcon(o.isNoShowIcon);
    rightPanelWidget->showIconForFyleSystem(o.isShowIconForFilesystem);
    rightPanelWidget->showIconForVirtualFolder(o.isShowOverlayIcon);
    rightPanelWidget->fileIconSize(o.fileIconSize);

    if(isRefresh)
    {
        leftPanelWidget->refresh();
        rightPanelWidget->refresh();
    }
}

void MainWindow::updateFonts(FontOption const& o)
{
    QFont font = o.mainWindow.font();

    toolButtons->setButtonFont(font);
    commandBar->setDirFont(font);

    leftPanelWidget->setDriveFont(font);
    leftPanelWidget->fileFont(o.fileList.font());
    rightPanelWidget->setDriveFont(font);
    rightPanelWidget->fileFont(o.fileList.font());
}

void MainWindow::updateColors(ColorOption const& o, bool isRefresh)
{
    leftPanelWidget->setItemColor(o.fontColor,
                                  o.background1Color,
                                  o.background2Color);
    leftPanelWidget->setItemSelectedColor(o.background1Color,
                                          o.markColor,
                                          o.cursorColor);

    rightPanelWidget->setItemColor(o.fontColor,                                
                                   o.background1Color,
                                   o.background2Color);
    rightPanelWidget->setItemSelectedColor(o.background1Color,
                                           o.markColor,
                                           o.cursorColor);
    if(isRefresh)
    {
        leftPanelWidget->refresh();
        rightPanelWidget->refresh();
    }
}

void MainWindow::updateOperation(OperationOption const& o)
{
    leftPanelWidget->setGoToRoot(o.isGoToRootWhenChangeDrive);
    leftPanelWidget->setRenameFileName(o.isSelectFileNameWhenRenaming);

    rightPanelWidget->setGoToRoot(o.isGoToRootWhenChangeDrive);
    rightPanelWidget->setRenameFileName(o.isSelectFileNameWhenRenaming);
}
