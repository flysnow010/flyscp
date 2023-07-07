#include "passworddialog.h"
#include "ui_passworddialog.h"

PasswordDialog::PasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

}

PasswordDialog::~PasswordDialog()
{
    delete ui;
}

void PasswordDialog::setPromptText(QString const& text)
{
    ui->labelPrompt->setText(text);
}

QString PasswordDialog::password() const
{
    return ui->editPassword->text();
}

bool PasswordDialog::isSavePassword() const
{
    return ui->checkBoxIsSave->isChecked();
}
