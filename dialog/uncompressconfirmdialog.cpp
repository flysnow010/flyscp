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

QString UnCompressConfirmDialog::targetFileType() const
{
    return ui->comboBoxFileType->currentText();
}

QString UnCompressConfirmDialog::overwriteMode() const
{
    if(isAlongWithPath())
        return QString();

    if(ui->rbOverWrite->isChecked())
        return QString(" -aoa");
    else if(ui->rbSkip->isChecked())
        return QString(" -aos");
    else if(ui->rbAutoRename->isChecked())
        return QString(" -aot");
    return QString();
}

bool UnCompressConfirmDialog::isAlongWithPath() const
{
    return ui->cbAlongPath->isChecked();
}

bool UnCompressConfirmDialog::isSameNameSubFolder() const
{
    return ui->cbUncompressToSubFolderSameName->isChecked();
}


