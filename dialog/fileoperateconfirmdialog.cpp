#include "fileoperateconfirmdialog.h"
#include "ui_fileoperateconfirmdialog.h"

FileOperateConfirmDialog::FileOperateConfirmDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileOperateConfirmDialog)
{
    ui->setupUi(this);
}

FileOperateConfirmDialog::~FileOperateConfirmDialog()
{
    delete ui;
}

void FileOperateConfirmDialog::setLabel(QString const& label)
{
    ui->label->setText(label);
}
void FileOperateConfirmDialog::setPath(QString const& path)
{
    ui->lineEditPath->setText(path);
}

QString FileOperateConfirmDialog::fileType() const
{
    return ui->lineEditType->text();
}

