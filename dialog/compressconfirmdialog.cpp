#include "compressconfirmdialog.h"
#include "ui_compressconfirmdialog.h"

CompressConfirmDialog::CompressConfirmDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CompressConfirmDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

CompressConfirmDialog::~CompressConfirmDialog()
{
    delete ui;
}

void CompressConfirmDialog::setFileCount(int size)
{
    if(size == 1)
        ui->labelTip->setText("Compress these 1 file or folder folder to:");
    else
        ui->labelTip->setText(QString("Compress these %1 files or folders into:").arg(size));

}

void CompressConfirmDialog::setTargetFileName(QString const& fileName)
{
    ui->lineEditFileName->setText(fileName);
    ui->lineEditFileName->setFocus();
}

QString CompressConfirmDialog::targetFileName()
{
    return ui->lineEditFileName->text();
}
