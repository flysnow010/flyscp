#include "titlebarwidget.h"
#include "ui_titlebarwidget.h"
#include <QDebug>

TitleBarWidget::TitleBarWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleBarWidget)
{
    ui->setupUi(this);

    connect(ui->btnLibDir, SIGNAL(clicked()), this, SIGNAL(libDirButtonClicked()));
    connect(ui->btnFavoritesDir, SIGNAL(clicked()), this, SIGNAL(favoritesDirButtonCLicked()));
    connect(ui->btnHistoryDir, SIGNAL(clicked()), this, SIGNAL(historyDirButtonClicked()));
    connect(ui->labelTitle, SIGNAL(linkHovered(QString)), this, SLOT(linkHovered(QString)));
    setActived(false);
}

TitleBarWidget::~TitleBarWidget()
{
    delete ui;
}

void TitleBarWidget::setTitle(QString const& title)
{
    ui->labelTitle->setText(title);
}

void TitleBarWidget::setActived(bool isActived)
{
    if(isActived)
    {
        setStyleSheet("QLabel{background-color: #BFCDDB}\nQToolButton{margin: 0px}");
    }
    else
    {
        setStyleSheet("QLabel{background-color: #90BFCDDB}\nQToolButton{margin: 0px}");
    }
}

void TitleBarWidget::linkHovered(const QString &link)
{
    qDebug() << link;
}
