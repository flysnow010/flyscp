#include "permissionsdialog.h"
#include "ui_permissionsdialog.h"

#include "util/utils.h"
#include "ssh/fileinfo.h"

PermissionsDialog::PermissionsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PermissionsDialog)
    , permissions_(0)
    , isDir_(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(ui->buttonBox, &QDialogButtonBox::clicked,
            this, [=](QAbstractButton * button)
    {
        if(ui->buttonBox->buttonRole(button)
                == QDialogButtonBox::ApplyRole)
           QDialog::accept();
    });

    connect(ui->checkBoxUserRead, &QCheckBox::stateChanged,
            this, [=](int state)
    {
        if(state == Qt::Checked)
            permissions_ |= ssh::FileInfo::User_Read;
        else
            permissions_ &= (~ssh::FileInfo::User_Read);
        updatePermissions();
    });

    connect(ui->checkBoxUserWrite, &QCheckBox::stateChanged,
            this, [=](int state)
    {
        if(state == Qt::Checked)
            permissions_ |= ssh::FileInfo::User_Write;
        else
            permissions_ &= (~ssh::FileInfo::User_Write);
        updatePermissions();
    });

    connect(ui->checkBoxUserExecute, &QCheckBox::stateChanged,
            this, [=](int state)
    {
        if(state == Qt::Checked)
            permissions_ |= ssh::FileInfo::User_Exe;
        else
            permissions_ &= (~ssh::FileInfo::User_Exe);
        updatePermissions();
    });

    connect(ui->checkBoxGroupRead, &QCheckBox::stateChanged,
            this, [=](int state)
    {
        if(state == Qt::Checked)
            permissions_ |= ssh::FileInfo::Group_Read;
        else
            permissions_ &= (~ssh::FileInfo::Group_Read);
        updatePermissions();
    });

    connect(ui->checkBoxGroupWrite, &QCheckBox::stateChanged,
            this, [=](int state)
    {
        if(state == Qt::Checked)
            permissions_ |= ssh::FileInfo::Group_Write;
        else
            permissions_ &= (~ssh::FileInfo::Group_Write);
        updatePermissions();
    });

    connect(ui->checkBoxGroupExecute, &QCheckBox::stateChanged,
            this, [=](int state)
    {
        if(state == Qt::Checked)
            permissions_ |= ssh::FileInfo::Group_Exe;
        else
            permissions_ &= (~ssh::FileInfo::Group_Exe);
        updatePermissions();
    });

    connect(ui->checkBoxOtherRead, &QCheckBox::stateChanged,
            this, [=](int state)
    {
        if(state == Qt::Checked)
            permissions_ |= ssh::FileInfo::Other_Read;
        else
            permissions_ &= (~ssh::FileInfo::Other_Read);
        updatePermissions();
    });

    connect(ui->checkBoxOtherWrite, &QCheckBox::stateChanged,
            this, [=](int state)
    {
        if(state == Qt::Checked)
            permissions_ |= ssh::FileInfo::Other_Write;
        else
            permissions_ &= (~ssh::FileInfo::Other_Write);
        updatePermissions();
    });

    connect(ui->checkBoxOtherExecute, &QCheckBox::stateChanged,
            this, [=](int state)
    {
        if(state == Qt::Checked)
            permissions_ |= ssh::FileInfo::Other_Exe;
        else
            permissions_ &= (~ssh::FileInfo::Other_Exe);
        updatePermissions();
    });
}

PermissionsDialog::~PermissionsDialog()
{
    delete ui;
}

void PermissionsDialog::setFileName(QString const& fileName)
{
    ui->labelName->setText(QString(tr("%1 \"%2\""))
                           .arg(ui->labelName->text(), fileName));
}

void PermissionsDialog::setPermissions(quint32 permissions, bool isDir)
{
    permissions_ = permissions;
    isDir_ = isDir;
    ui->checkBoxRecursively->setVisible(isDir_);
    updatePermissions();
    adjustSize();
}

quint16 PermissionsDialog::permissions() const
{
    return permissions_ & ssh::FileInfo::ALlMask;
}

bool PermissionsDialog::isRecursively() const
{
    return ui->checkBoxRecursively->isChecked();
}

void PermissionsDialog::updatePermissions()
{
    ui->lineEditPermissionsText->setText(Utils::permissionsText(permissions_, isDir_));
    ui->checkBoxUserRead->setChecked(permissions_ & ssh::FileInfo::User_Read);
    ui->checkBoxUserWrite->setChecked(permissions_ & ssh::FileInfo::User_Write);
    ui->checkBoxUserExecute->setChecked(permissions_ & ssh::FileInfo::User_Exe);
    ui->checkBoxGroupRead->setChecked(permissions_ & ssh::FileInfo::Group_Read);
    ui->checkBoxGroupWrite->setChecked(permissions_ & ssh::FileInfo::Group_Write);
    ui->checkBoxGroupExecute->setChecked(permissions_ & ssh::FileInfo::Group_Exe);
    ui->checkBoxOtherRead->setChecked(permissions_ & ssh::FileInfo::Other_Read);
    ui->checkBoxOtherWrite->setChecked(permissions_ & ssh::FileInfo::Other_Write);
    ui->checkBoxOtherExecute->setChecked(permissions_ & ssh::FileInfo::Other_Exe);
    ui->lineEditPermissionsOctal->setText(QString::number(permissions_ & ssh::FileInfo::ALlMask, 8));
}
