#include "serchfiledialog.h"
#include "ui_serchfiledialog.h"

SerchFileDialog::SerchFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerchFileDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);
}

SerchFileDialog::~SerchFileDialog()
{
    delete ui;
}

void SerchFileDialog::setSearchPath(QString  const& filePath)
{
    ui->cbFolder->setCurrentText(filePath);
}
