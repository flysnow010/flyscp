#include "uncompressconfirmdialog.h"
#include "ui_uncompressconfirmdialog.h"

UnCompressConfirmDialog::UnCompressConfirmDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UnCompressConfirmDialog)
{
    ui->setupUi(this);
}

UnCompressConfirmDialog::~UnCompressConfirmDialog()
{
    delete ui;
}
