#include "panelwidget.h"
#include "ui_panelwidget.h"
#include "view/localdirdockwidget.h"
#include "util/utils.h"
#include "core/dirhistory.h"
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
    , dirHistory(new DirHistory())
{
    ui->setupUi(this);
    ui->tabWidget->setTabBarAutoHide(true);
    updateDrivers();
    connect(buttonGroup, SIGNAL(buttonToggled(QAbstractButton*,bool)),
            this, SLOT(dirverChanged(QAbstractButton*,bool)));
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &PanelWidget::currentChanged);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &PanelWidget::tabCloseRequested);
}

PanelWidget::~PanelWidget()
{
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
    settings.beginWriteArray("dirNames", dirNames.size());
    for(int i = 0; i < dirNames.size(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("dirName", dirNames[i]);
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
    int size = settings.beginReadArray("dirNames");
    for(int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        dirNames << settings.value("dirName").toString();
    }
    settings.endArray();
    settings.endGroup();
    dirHistory->setDirs(dirNames);
}

void PanelWidget::addDirToHistory(QString const& dir, bool isRemote)
{
    Q_UNUSED(isRemote)
    dirHistory->add(dir);
}

void PanelWidget::libDirContextMenu()
{
    QMenu menu;
    menu.exec(QCursor::pos());
}

void PanelWidget::favoritesDirContextMenu()
{
    QMenu menu;
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
            if(filePath.startsWith(buttons[i]->text()))
            {
                isChecked = false;
                buttons[i]->setChecked(true);
                break;
            }
        }
    }
}

void PanelWidget::updateDrivers()
{
    QFileInfoList drivers = QDir::drives();
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setMargin(3);
    layout->setSpacing(5);

    QFileIconProvider fip;
    for(int i = 0; i < drivers.size(); i++)
    {
        QToolButton* button = new QToolButton();
        button->setCheckable(true);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(drivers[i].path().at(0));
        button->setIcon(fip.icon(drivers[i]));
        layout->addWidget(button);
        buttonGroup->addButton(button);
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

void PanelWidget::dirverChanged(QAbstractButton* button, bool checked)
{
    if(checked)
    {
        if(isChecked)
            updateDir(button->text());
        else
            isChecked = true;
    }
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
