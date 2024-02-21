#include "compressconfirmdialog.h"
#include "ui_compressconfirmdialog.h"
#include <QDir>
#include <QFileInfo>

#define ARRAY_SIZE(Array) (sizeof(Array) / sizeof(Array[0]))

QString CompressConfirmDialog::currentSuffix(".zip");

CompressConfirmDialog::CompressConfirmDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CompressConfirmDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->comboBoxVolumeSize->addItem("10M", 10);
    ui->comboBoxVolumeSize->addItem("100M", 100);
    ui->comboBoxVolumeSize->addItem("1000M", 1000);
    ui->comboBoxVolumeSize->addItem("650M - CD", 650);
    ui->comboBoxVolumeSize->addItem("700M - CD", 700);
    ui->comboBoxVolumeSize->addItem("4092M - FAT", 4092);
    ui->comboBoxVolumeSize->addItem("4480M - DVD", 4480);
    ui->comboBoxVolumeSize->addItem("8128M - DVD DL", 8128);
    ui->comboBoxVolumeSize->addItem("23040M - BD", 23040);

    QRadioButton* radioButtons[] = {
        ui->rbZip, ui->rb7z, ui->rbWim, ui->rbTar,
        ui->rbGZ, ui->rbXZ, ui->rbBZ2,
        ui->rbTarGz, ui->rbTarXz, ui->rbTarBz2
    };

    for(quint16 i = 0; i < ARRAY_SIZE(radioButtons); i++)
    {
        QString suffix = QString(".%1").arg(radioButtons[i]->text());
        if(suffix == currentSuffix)
        {
            radioButtons[i]->setChecked(true);
            if(suffix == ".7z")
            {
                ui->cbCreateSFX->setEnabled(true);
                ui->rbConsoleSFX->setEnabled(true);
                ui->rbGUISFX->setEnabled(true);
            }
            break;
        }
    }
}

CompressConfirmDialog::~CompressConfirmDialog()
{
    delete ui;
}

void CompressConfirmDialog::setConnect()
{
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
    connect(ui->rbTarXz, &QAbstractButton::clicked, this, [&](bool isChecked){
        if(isChecked)
            changeSuffix(".tar.xz");
    });
    connect(ui->rbTarBz2, &QAbstractButton::clicked, this, [&](bool isChecked){
        if(isChecked)
            changeSuffix(".tar.bz2");
    });
    connect(ui->cbCreateSFX, &QAbstractButton::clicked, this, [&](bool isChecked){
        if(isChecked)
        {
            QFileInfo fileInfo(ui->lineEditFileName->text());
            QString fileName = fileInfo.dir().filePath(baseFileName + ".exe");
            ui->lineEditFileName->setText(fileName);
        }
    });
}

void CompressConfirmDialog::changeSuffix(QString const& suffix,
                                         bool isCanEncryption)
{
    QFileInfo fileInfo(ui->lineEditFileName->text());
    QString fileName = fileInfo.dir().filePath(baseFileName + suffix);
    ui->lineEditFileName->setText(fileName);
    if(isCanEncryption)
        ui->cbEncryption->setEnabled(true);
    else
    {
        ui->cbEncryption->setEnabled(false);
        ui->cbEncryption->setChecked(false);
    }

    if(suffix == ".7z")
    {
        ui->cbCreateSFX->setEnabled(true);
        ui->rbConsoleSFX->setEnabled(true);
        ui->rbGUISFX->setEnabled(true);
    }
    else
    {
        ui->cbCreateSFX->setEnabled(false);
        ui->rbConsoleSFX->setEnabled(false);
        ui->rbGUISFX->setEnabled(false);
        ui->cbCreateSFX->setChecked(false);
    }
    currentSuffix = suffix;
}

void CompressConfirmDialog::setLabel(int size)
{
    if(size == 1)
        ui->labelTip->setText(tr("Compress these 1 file or folder folder to:"));
    else
        ui->labelTip->setText(QString(tr("Compress these %1 files or folders into:")).arg(size));
}

void CompressConfirmDialog::setFileNames(QStringList const& fileNames,
                  QString const& targetPath)
{
    setLabel(fileNames.size());
    bool isDir = false;
    foreach(auto const& fileName, fileNames)
    {
        if(QFileInfo(fileName).isDir())
        {
            ui->rbBZ2->hide();
            ui->rbGZ->hide();
            ui->rbXZ->hide();
            isDir = true;
            break;;
        }
    }
    if(!isDir)
    {
        ui->rbTarGz->hide();
        ui->rbTarXz->hide();
        ui->rbTarBz2->hide();
    }

    QFileInfo fileInfo(fileNames[0]);

    if(fileNames.size() > 1)
    {
        baseFileName = QFileInfo(fileInfo.path()).fileName();
    }
    else
    {
        if(fileInfo.isDir())
            baseFileName = fileInfo.fileName();
        else
        {
            QString fileName = fileInfo.fileName().toLower();
            if(fileName.endsWith(".tar.gz")
                    || fileName.endsWith(".tar.gz")
                    || fileName.endsWith(".tar.xz"))
                baseFileName = fileInfo.fileName();
            else
                baseFileName = fileInfo.completeBaseName();
        }
    }
    QString dstFileName = baseFileName + CompressConfirmDialog::currentSuffix;
    ui->lineEditFileName->setText(QDir(targetPath).filePath(dstFileName));
    ui->lineEditFileName->setFocus();
    setConnect();
}

QString CompressConfirmDialog::targetFileName()
{
    return ui->lineEditFileName->text();
}

void CompressConfirmDialog::setSettings(CompressParam const& param)
{
    ui->cbAlongPath->setChecked(param.isWithPath);
    ui->cbRecursively->setChecked(param.isRecursively);
    ui->cbMultiVolume->setChecked(param.isMultiVolume);
    ui->cbMoveCompress->setChecked(param.isMoveFile);
    ui->cbCreateSFX->setChecked(param.isCreateSFX);
    ui->rbGUISFX->setChecked(param.isGuiSFX);
    ui->cbCompressSingle->setChecked(param.isSignle);
    ui->cbEncryption->setChecked(param.isEncryption);
    ui->comboBoxFilter->setCurrentText(param.filter);
}

CompressParam CompressConfirmDialog::settings() const
{
    CompressParam param;
    param.isWithPath = ui->cbAlongPath->isChecked();
    param.isRecursively = ui->cbRecursively->isChecked();
    param.isMultiVolume = ui->cbMultiVolume->isChecked();
    param.isMoveFile = ui->cbMoveCompress->isChecked();
    param.isCreateSFX = ui->cbCreateSFX->isChecked();
    param.isGuiSFX = ui->rbGUISFX->isChecked();
    param.isSignle = ui->cbCompressSingle->isChecked();
    param.isEncryption = ui->cbEncryption->isChecked();
    param.password = ui->lineEditPassword->text();
    param.filter = ui->comboBoxFilter->currentText();
    param.suffix = currentSuffix;
    param.volumeSize = ui->comboBoxVolumeSize->currentData().toUInt();
    return param;
}
