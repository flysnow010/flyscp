#include "titlebarwidget.h"
#include "ui_titlebarwidget.h"
#include <QDebug>
#include <QScreen>
#include <QMouseEvent>
#include <QFontMetrics>

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
    ui->labelTitle->installEventFilter(this);
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

bool TitleBarWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mounseEvent = static_cast<QMouseEvent *>(event);
        if(mounseEvent->x() > 6)
            selectDir(mounseEvent->x() - 6);
    }
    return QWidget::eventFilter(obj, event);
}

void TitleBarWidget::selectDir(int x)
{
    QFontMetricsF fontmetrics(ui->labelTitle->font());
    QString text = ui->labelTitle->text();
    int width = 0,i;
    for(i= 0; i < text.size(); i++)
    {
        width += fontmetrics.width(text[i]);
        if(width > x)
            break;
    }
    if(text[i] == QChar('\\') || text[i] == QChar(':'))
        return;
    for(; i < text.size(); i++)
    {
        if(text[i] == QChar('\\') || text[i] == QChar(':'))
            break;
    }
    if(width > x)
    {
        if(i == 1)
            emit dirSelected(QString("%1:/").arg(text.left(i)));
        else
            emit dirSelected(text.left(i));
    }
}

