#include "propertydialog.h"
#include "ui_propertydialog.h"
#include "util/utils.h"
#include <QDateTime>

PropertyDialog::PropertyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertyDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

PropertyDialog::~PropertyDialog()
{
    delete ui;
}

void PropertyDialog::setFileInfo(ssh::FileInfoPtr const& fileInfo)
{
    if(fileInfo->is_dir())
    {
        ui->labelName->setText(QString("Folder name: %1").arg(QString::fromStdString(fileInfo->name())));
        ui->labelFileSize->hide();
    }
    else
    {
        ui->labelName->setText(QString("File name: %1").arg(QString::fromStdString(fileInfo->name())));
        ui->labelFileSize->setText(QString("%1 %2")
                               .arg(ui->labelFileSize->text(),
                                    Utils::formatFileSize(fileInfo->size())));
    }
    ui->labelDate->setText(QString("%1 %2")
                           .arg(ui->labelDate->text(),
                                QDateTime::fromSecsSinceEpoch(fileInfo->time()).toString("yyyy-MM-dd HH:mm:ss")));
    ui->labelOwer->setText(QString("%1   %2")
                            .arg(ui->labelOwer->text(),
                                 QString::fromStdString(fileInfo->owner())));
    ui->labelGroup->setText(QString("%1  %2")
                            .arg(ui->labelGroup->text(),
                                 QString::fromStdString(fileInfo->group())));
    ui->labelPermissions->setText(QString("%1 %2")
                                  .arg(ui->labelPermissions->text(),
                                       Utils::permissionsText(fileInfo->permissions(), fileInfo->is_dir())));
}
