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

void UnCompressConfirmDialog::setSettings(UncompressParam const& param)
{
    ui->cbAlongPath->setChecked(param.isWithPath);
    ui->cbCreateDir->setChecked(param.isCreateDir);
    if(param.mode == UncompressParam::OverWrite)
        ui->rbOverWrite->setChecked(true);
    else if(param.mode == UncompressParam::Skip)
        ui->rbSkip->setChecked(true);
    else if(param.mode == UncompressParam::AutoRename)
        ui->rbAutoRename->setChecked(true);
    ui->comboBoxFilter->setCurrentText(param.filter);
}

UncompressParam UnCompressConfirmDialog::settings() const
{
    UncompressParam param;

    param.isWithPath = ui->cbAlongPath->isChecked();
    param.isCreateDir = ui->cbCreateDir->isChecked();
    if(ui->rbOverWrite->isChecked())
        param.mode = UncompressParam::OverWrite;
    else if(ui->rbSkip->isChecked())
        param.mode = UncompressParam::Skip;
    else if(ui->rbAutoRename->isChecked())
        param.mode = UncompressParam::AutoRename;
    param.filter = ui->comboBoxFilter->currentText();

    return param;
}

QString UnCompressConfirmDialog::targetPath() const
{
    return ui->lineEditPath->text();
}



