#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "model/localfilemodel.h"
#include "view/panelwidget.h"
#include "view/localdirdockwidget.h"
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
{
    ui->setupUi(this);
    QSplitter *spliter = new QSplitter(this);
    spliter->setHandleWidth(0);
    spliter->addWidget(leftPanelWidget);
    spliter->addWidget(rightPanelWidget);

    LocalDirDockWidget* leftDirView = new LocalDirDockWidget(this);
    leftDirView->setDir("d:/test");
    leftPanelWidget->addDir(leftDirView, "d:/test");
    LocalDirDockWidget* rightDirView = new LocalDirDockWidget(this);
    rightDirView->setDir("e:/Software");
    rightPanelWidget->addDir(rightDirView, "e:/Software");
    setCentralWidget(spliter);
    loadSettings();

    QMenuBar *bar = new QMenuBar(ui->menubar);

    QMenu *menuHelp = new QMenu("?", bar);
    bar->addMenu(menuHelp);
    menuHelp->addAction(ui->actionAbout);

    ui->menubar->setCornerWidget(bar);
    createConnects();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

void MainWindow::createConnects()
{
    connect(ui->actionAbout,  &QAction::triggered, [=](bool){
        AboutDialog dialog;
        dialog.exec();
    });
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

