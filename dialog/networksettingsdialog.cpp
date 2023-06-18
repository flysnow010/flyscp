#include "networksettingsdialog.h"
#include "ui_networksettingsdialog.h"

NetworkSettingsDialog::NetworkSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetworkSettingsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

NetworkSettingsDialog::~NetworkSettingsDialog()
{
    delete ui;
}
