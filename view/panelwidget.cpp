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
    //ui->tabWidget->setTabBarAutoHide(true);
    updateDrivers();
    connect(buttonGroup, SIGNAL(buttonToggled(QAbstractButton*, bool)),
            this, SLOT(dirverChanged(QAbstractButton*, bool)));
}

PanelWidget::~PanelWidget()
{
    delete ui;
}

void PanelWidget::addLocalDir(QWidget* widget, QString const& text)
{
    ui->tabWidget->addTab(widget, Utils::driverIcon(), text);
}

void PanelWidget::addremoteDir(QWidget* widget, QString const& text)
{
    ui->tabWidget->addTab(widget, Utils::networkIcon(), text);
}

void PanelWidget::updateTexts(QWidget* widget)
{
    LocalDirDockWidget* localDirWidget = dynamic_cast<LocalDirDockWidget *>(widget);
    if(localDirWidget)
    {
        QList<QAbstractButton*> buttons = buttonGroup->buttons();
        QString dir = localDirWidget->dir().toUpper();
        for(int i = 0; i < buttons.size(); i++)
        {
            if(dir.startsWith(buttons[i]->text()))
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

    QToolButton* rootButton = new QToolButton();
    rootButton->setText("/");
    layout->addWidget(rootButton);

    QToolButton *topButton = new QToolButton();
    topButton->setText("..");
    layout->addWidget(topButton);

    connect(rootButton, SIGNAL(clicked()), this, SLOT(backToRoot()));
    connect(topButton, SIGNAL(clicked()), this, SLOT(backToPrePath()));
    layout->addStretch();
    ui->driversWidget->setLayout(layout);

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
    LocalDirDockWidget* localDirWidget = dynamic_cast<LocalDirDockWidget *>(ui->tabWidget->currentWidget());
    if(localDirWidget)
    {
        QAbstractButton* button = buttonGroup->checkedButton();
        if(button)
            updateDir(button->text());
    }
}

void PanelWidget::backToPrePath()
{
    LocalDirDockWidget* localDirWidget = dynamic_cast<LocalDirDockWidget *>(ui->tabWidget->currentWidget());
    if(localDirWidget)
        localDirWidget->cd("..");
}

void PanelWidget::updateDir(QString const& driver)
{
    LocalDirDockWidget* localDirWidget = dynamic_cast<LocalDirDockWidget *>(ui->tabWidget->currentWidget());
    if(localDirWidget)
        localDirWidget->setDir(driver + ":/");
}
