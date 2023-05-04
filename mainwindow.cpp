#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "model/localdirmodel.h"
#include "view/panelwidget.h"
#include "view/localdirdockwidget.h"
#include "view/remotedockwidget.h"
#include "dialog/connectdialog.h"
#include "dialog/aboutdialog.h"
#include <QSplitter>
#include <QTabWidget>
#include <QSettings>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , leftPanelWidget(new PanelWidget(this))
    , rightPanelWidget(new PanelWidget(this))
    , leftDirView(new LocalDirDockWidget(this))
    , rightDirView(new LocalDirDockWidget(this))
{
    ui->setupUi(this);
    QSplitter *spliter = new QSplitter(this);
    spliter->setHandleWidth(0);
    spliter->addWidget(leftPanelWidget);
    spliter->addWidget(rightPanelWidget);

    leftPanelWidget->addLocalDir(leftDirView);
    rightPanelWidget->addLocalDir(rightDirView);
    setCentralWidget(spliter);


    QMenuBar *bar = new QMenuBar(ui->menubar);

    QMenu *menuHelp = new QMenu("?", bar);
    bar->addMenu(menuHelp);
    menuHelp->addAction(ui->actionAbout);

    ui->menubar->setCornerWidget(bar);

    load();
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
        //settings.hostName = "192.168.3.197";
        settings.hostName = "13.13.13.159";
        settings.userName = "james";
        dialog.setType(ConnectType::SSH);
        dialog.setSshSettings(settings);
        if(dialog.exec() == QDialog::Accepted)
        {
            RemoteDockWidget* rightDirView = new RemoteDockWidget(this);
            settings.passWord = "james010";
            rightDirView->start(settings);
            rightPanelWidget->addRemoteDir(rightDirView);
        }
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
}

void MainWindow::save()
{
    saveSettings();
    leftDirView->saveSettings("LeftDirView");
    rightDirView->saveSettings("RightDirView");
}

void MainWindow::load()
{
    loadSettings();
    leftDirView->loadSettings("LeftDirView");
    rightDirView->loadSettings("RightDirView");
    leftPanelWidget->updateTexts(leftDirView);
    rightPanelWidget->updateTexts(rightDirView);
}

void MainWindow::saveSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
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
}

