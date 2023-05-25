#include "compressconfirmdialog.h"
#include "ui_compressconfirmdialog.h"

CompressConfirmDialog::CompressConfirmDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CompressConfirmDialog)
{
    ui->setupUi(this);
}

CompressConfirmDialog::~CompressConfirmDialog()
{
    delete ui;
}
