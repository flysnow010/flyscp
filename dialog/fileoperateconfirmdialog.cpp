#include "fileoperateconfirmdialog.h"
#include "ui_fileoperateconfirmdialog.h"
#include "util/utils.h"

FileOperateConfirmDialog::FileOperateConfirmDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileOperateConfirmDialog)
{
    ui->setupUi(this);
    connect(ui->btnBrowsePath, &QToolButton::clicked, [=]{
        QString path = Utils::getPath("Select a path", ui->lineEditPath->text());
        if(!path.isEmpty())
            ui->lineEditPath->setText(path);
    });
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

QString FileOperateConfirmDialog::path() const
{
    return ui->lineEditPath->text();
}

QString FileOperateConfirmDialog::fileType() const
{
    return ui->lineEditType->text();
}

