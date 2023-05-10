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
    ui->localDrivers->hide();
    ui->remoteDrivers->hide();
    updateLocalDrivers();
    remoteDirvers();
    connect(buttonGroup, SIGNAL(buttonToggled(QAbstractButton*,bool)),
            this, SLOT(dirverChanged(QAbstractButton*,bool)));
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &PanelWidget::currentChanged);
}

PanelWidget::~PanelWidget()
{
    delete ui;
}

void PanelWidget::addDirTab(QWidget* widget, QIcon const& icon, QString const& text)
{
    ui->tabWidget->addTab(widget, icon, text);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
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

void PanelWidget::updateLocalDrivers()
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

    QToolButton* rootButton = new QToolButton();
    rootButton->setText("/");
    layout->addWidget(rootButton);

    QToolButton *topButton = new QToolButton();
    topButton->setText("..");
    layout->addWidget(topButton);

    connect(rootButton, SIGNAL(clicked()), this, SLOT(backToRoot()));
    connect(topButton, SIGNAL(clicked()), this, SLOT(backToPrePath()));
    layout->addStretch();
    ui->localDrivers->setLayout(layout);
}

void PanelWidget::remoteDirvers()
{
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setMargin(3);
    layout->setSpacing(5);

    QToolButton* homeButton = new QToolButton();
    homeButton->setIcon(Utils::computerIcon());
    layout->addWidget(homeButton);

    QToolButton* rootButton = new QToolButton();
    rootButton->setText("/");
    layout->addWidget(rootButton);

    QToolButton *topButton = new QToolButton();
    topButton->setText("..");
    layout->addWidget(topButton);

    connect(rootButton, SIGNAL(clicked()), this, SLOT(backToRoot()));
    connect(topButton, SIGNAL(clicked()), this, SLOT(backToPrePath()));
    layout->addStretch();
    ui->remoteDrivers->setLayout(layout);
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

void PanelWidget::backToRoot()
{
    QAbstractButton* button = buttonGroup->checkedButton();
    if(button)
        updateDir(button->text());
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

    ui->localDrivers->setVisible(!dir->isRemote());
    ui->remoteDrivers->setVisible(dir->isRemote());

}
