#include "panelwidget.h"
#include "ui_panelwidget.h"
#include "view/localdirdockwidget.h"
#include "util/utils.h"
#include <QDir>
#include <QHBoxLayout>
#include <QToolButton>
#include <QFileIconProvider>
#include <QButtonGroup>

PanelWidget::PanelWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PanelWidget)
    , buttonGroup(new QButtonGroup(this))
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
        dir->setDir(driver + ":/");
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
