#include "titlebarwidget.h"
#include "ui_titlebarwidget.h"
#include <QDebug>
#include <QScreen>
#include <QMouseEvent>
#include <QFontMetrics>

TitleBarWidget::TitleBarWidget(bool isWindows, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TitleBarWidget)
    , actived_(false)
    , isWindows_(isWindows)
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
    if(isWindows_)
        ui->labelTitle->setText(title.split("/").join("\\"));
    else
        ui->labelTitle->setText(title);
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
        emit actived();
    }
    return QWidget::eventFilter(obj, event);
}

void TitleBarWidget::selectDir(int x)
{
    QString text = ui->labelTitle->text();
    if(text.startsWith("\\\\")) // \\桌面
        return;

    QFontMetricsF fontmetrics(ui->labelTitle->font());
    int width = 0,i;
    for(i= 0; i < text.size(); i++)
    {
        width += fontmetrics.width(text[i]);
        if(width > x)
            break;
    }
    if(isWindows_)
    {
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
    else
    {
        if(i == 0 && text[i] != QChar('/'))
            return;
        for(; i < text.size(); i++)
        {
            if(text[i] == QChar('/'))
                break;
        }
        if(width > x)
        {
            if(i == 0)
                emit dirSelected(text.left(i+1));
            else
                emit dirSelected(text.left(i));
        }
    }
}

