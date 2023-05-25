#include "uncompressconfirmdialog.h"
#include "ui_uncompressconfirmdialog.h"

UnCompressConfirmDialog::UnCompressConfirmDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UnCompressConfirmDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

UnCompressConfirmDialog::~UnCompressConfirmDialog()
{
    delete ui;
}

void UnCompressConfirmDialog::setTargetPath(QString const& filePath)
{
    ui->lineEditPath->setText(filePath);
    ui->lineEditPath->setFocus();
}

QString UnCompressConfirmDialog::targetPath() const
{
    return ui->lineEditPath->text();
}

QString UnCompressConfirmDialog::targetFileName() const
{
    return ui->comboBoxFileName->currentText();
}

bool UnCompressConfirmDialog::isAlongWithPath() const
{
    return ui->cbAlongPath->isChecked();
}

bool UnCompressConfirmDialog::isOverwrite() const
{
    return ui->cbOverWrite->isChecked();
}
