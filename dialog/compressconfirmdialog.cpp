#include "compressconfirmdialog.h"
#include "ui_compressconfirmdialog.h"
#include <QDir>
#include <QFileInfo>

CompressConfirmDialog::CompressConfirmDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CompressConfirmDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(ui->rbZip, &QAbstractButton::clicked, this, [&](bool isChecked){
        if(isChecked)
            changeSuffix(".zip", true);
    });
    connect(ui->rb7z, &QAbstractButton::clicked, this, [&](bool isChecked){
        if(isChecked)
            changeSuffix(".7z", true);
    });
    connect(ui->rbWim, &QAbstractButton::clicked, this, [&](bool isChecked){
        if(isChecked)
            changeSuffix(".wim", true);
    });
    connect(ui->rbTar, &QAbstractButton::clicked, this, [&](bool isChecked){
        if(isChecked)
            changeSuffix(".tar");
    });

    connect(ui->rbGZ, &QAbstractButton::clicked, this, [&](bool isChecked){
        if(isChecked)
            changeSuffix(".gz");
    });
    connect(ui->rbXZ, &QAbstractButton::clicked, this, [&](bool isChecked){
        if(isChecked)
            changeSuffix(".xz");
    });
    connect(ui->rbBZ2, &QAbstractButton::clicked, this, [&](bool isChecked){
        if(isChecked)
            changeSuffix(".bz2");
    });
    connect(ui->rbTarGz, &QAbstractButton::clicked, this, [&](bool isChecked){
        if(isChecked)
            changeSuffix(".tar.gz");
    });

    ui->comboBoxVolumeSize->addItem("10M");
    ui->comboBoxVolumeSize->addItem("100M");
    ui->comboBoxVolumeSize->addItem("1000M");
    ui->comboBoxVolumeSize->addItem("650M - CD");
    ui->comboBoxVolumeSize->addItem("700M - CD");
    ui->comboBoxVolumeSize->addItem("4092M - FAT");
    ui->comboBoxVolumeSize->addItem("4480M - DVD");
    ui->comboBoxVolumeSize->addItem("8128M - DVD DL");
    ui->comboBoxVolumeSize->addItem("23040M - BD");
}

CompressConfirmDialog::~CompressConfirmDialog()
{
    delete ui;
}

void CompressConfirmDialog::changeSuffix(QString const& suffix, bool isCanEncryption)
{
    QFileInfo fileInfo(ui->lineEditFileName->text());
    QString fileName = fileInfo.dir().filePath(fileInfo.baseName() + suffix);
    ui->lineEditFileName->setText(fileName);
    if(isCanEncryption)
        ui->cbEncryption->setEnabled(true);
    else
    {
        ui->cbEncryption->setEnabled(false);
        ui->cbEncryption->setChecked(false);
    }
}

void CompressConfirmDialog::setLabel(int size)
{
    if(size == 1)
        ui->labelTip->setText("Compress these 1 file or folder folder to:");
    else
        ui->labelTip->setText(QString("Compress these %1 files or folders into:").arg(size));
}



void CompressConfirmDialog::setFileNames(QStringList const& fileNames)
{
    setLabel(fileNames.size());
    foreach(auto const& fileName, fileNames)
    {
        if(QFileInfo(fileName).isDir())
        {
            ui->rbBZ2->hide();
            ui->rbGZ->hide();
            ui->rbXZ->hide();
            return;
        }
    }
    ui->rbTarGz->hide();
    ui->rbTarXz->hide();
    ui->rbTarBz2->hide();
}

void CompressConfirmDialog::setTargetFileName(QString const& fileName)
{
    ui->lineEditFileName->setText(fileName);
    ui->lineEditFileName->setFocus();
}

QString CompressConfirmDialog::targetFileName()
{
    return ui->lineEditFileName->text();
}

bool CompressConfirmDialog::isAlongWithPath() const
{
    return ui->cbAlongPath->isChecked();
}

bool CompressConfirmDialog::isRecursively() const
{
    return ui->cbRecursively->isChecked();
}

bool CompressConfirmDialog::isCreateSFX() const
{
    return ui->cbCreateSFX->isChecked();
}

bool CompressConfirmDialog::isCompressSignle() const
{
    return ui->cbCompressSingle->isChecked();
}

bool CompressConfirmDialog::isEncryption() const
{
    return ui->cbEncryption->isChecked();
}

QString CompressConfirmDialog::password() const
{
    ;
}
