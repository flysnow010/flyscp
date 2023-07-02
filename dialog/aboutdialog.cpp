#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->buildLabel->setText(QString(tr("Build on %1 %2")).arg(__DATE__, __TIME__));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
