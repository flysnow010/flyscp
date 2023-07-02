#include "connectdialog.h"
#include "ui_connectdialog.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QFileDialog>
#include <QPushButton>

ConnectDialog::ConnectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(ui->toolButtonAddFile, SIGNAL(clicked()),
            this, SLOT(selectPrivateKeyFileName()));
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}

ConnectType ConnectDialog::type() const
{
    int currentIndex = ui->tabWidget->currentIndex();
    if(ui->tabWidget->tabText(currentIndex) == "SSH")
        return SSH;

    return None;
}

void ConnectDialog::setType(ConnectType t)
{
    if(t == SSH)
        ui->tabWidget->setCurrentIndex(0);
}

SSHSettings::Ptr ConnectDialog::sshSettings()
{
    if(!settings_)
        settings_ = SSHSettings::Ptr(new SSHSettings);

    settings_->hostName = ui->sshHostname->text();
    settings_->userName = ui->sshUsername->text();
    settings_->port = ui->sshPort->value();

    if(ui->usePrivateKey)
    {
        settings_->privateKeyFileName = ui->privateKeyFileName->text();
        if(!settings_->privateKeyFileName.isEmpty())
            settings_->usePrivateKey = true;
    }

    if(settings_->name.isEmpty())
        settings_->name = QString("%1@%2").arg(settings_->userName, settings_->hostName);

    return settings_;
}

void ConnectDialog::setSshSettings(SSHSettings::Ptr const& settings)
{
    ui->sshHostname->setText(settings->hostName);
    ui->sshUsername->setText(settings->userName);
    ui->sshPort->setValue(settings->port);
    ui->usePrivateKey->setChecked(settings->usePrivateKey);
    ui->privateKeyFileName->setText(settings->privateKeyFileName);
    settings_ = settings;

    if(!settings_->name.isEmpty())
        setWindowTitle(QString(tr("Connect - %1")).arg(settings_->name));
}

void ConnectDialog::selectPrivateKeyFileName()
{
    static QString filePath;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Add Private Key File"),
                                                    filePath,
                                                    tr("KeyFile (*.*)"));
    if(!fileName.isEmpty())
    {
        filePath = QFileInfo(fileName).filePath();
        ui->privateKeyFileName->setText(fileName);
    }
}
