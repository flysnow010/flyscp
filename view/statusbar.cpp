#include "statusbar.h"
#include "ui_statusbar.h"

StatusBar::StatusBar(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::StatusBar)
{
    ui->setupUi(this);
    space = layout()->spacing() / 2 + layout()->margin();
}

StatusBar::~StatusBar()
{
    delete ui;
}

void StatusBar::setLeftWidth(int width)
{
    ui->labelLeft->setMaximumWidth(width - space);
    ui->labelLeft->setMinimumWidth(width - space);
}

void StatusBar::setLeftStatusText(QString const& text)
{
    ui->labelLeft->setText(text);
}

void StatusBar::setRightStatusText(QString const& text)
{
    ui->labelRight->setText(text);
}
