#include "titlebarwidget.h"
#include "ui_titlebarwidget.h"
#include <QDebug>
#include <QScreen>

TitleBarWidget::TitleBarWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TitleBarWidget)
    , actived_(false)
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
    QString newTitle = title.split("/").join("\\");
    ui->labelTitle->setText(newTitle);
}

void TitleBarWidget::setActived(bool isActived)
{
    actived_ = isActived;
    if(isActived)
        setStyleSheet("QLabel{"
                      "background-color: #FFBFCDDB;"
                      "margin: 0px;"
                      "padding-left: 2px;"
                      "border-width: 0px;"
                      "border-radius: 2px;}\n"
                      "QToolButton{"
                      "margin: 0px;"
                      "padding: 0px;"
                      "border-width: 0px;"
                      "border-radius: 0px;}");
    else
        setStyleSheet("QLabel{"
                      "background-color: #90BFCDDB;"
                      "margin: 0px;"
                      "padding-left: 2px;"
                      "border-width: 0px;"
                      "border-radius: 2px;}\n"
                      "QToolButton{"
                      "margin: 0px;"
                      "padding: 0px;"
                      "border-width: 0px;"
                      "border-radius: 0px;}");
}

void TitleBarWidget::linkHovered(const QString &link)
{
    qDebug() << link;
}
