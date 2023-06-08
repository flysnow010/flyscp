#include "connectdialog.h"
#include "ui_connectdialog.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QFileDialog>

ConnectDialog::ConnectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(ui->toolButtonAddFile, SIGNAL(clicked()), this, SLOT(selectPrivateKeyFileName()));
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

SSHSettings::Ptr ConnectDialog::sshSettings() const
{
    SSHSettings::Ptr settings(new SSHSettings);
    settings->hostName = ui->sshHostname->text();
    settings->userName = ui->sshUsername->text();
    settings->port = ui->sshPort->value();
    if(ui->usePrivateKey)
    {
        settings->privateKeyFileName = ui->privateKeyFileName->text();
        if(!settings->privateKeyFileName.isEmpty())
            settings->usePrivateKey = true;
    }
    settings->name = QString("%1@%2").arg(settings->userName, settings->hostName);
    return settings;
}

void ConnectDialog::setSshSettings(SSHSettings::Ptr const& settings)
{
    ui->sshHostname->setText(settings->hostName);
    ui->sshUsername->setText(settings->userName);
    ui->sshPort->setValue(settings->port);
    ui->usePrivateKey->setChecked(settings->usePrivateKey);
    ui->privateKeyFileName->setText(settings->privateKeyFileName);
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
