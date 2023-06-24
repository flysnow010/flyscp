#include "networksettingsdialog.h"
#include "ui_networksettingsdialog.h"
#include "model/netsettingsmodel.h"
#include "connectdialog.h"
#include <QSettings>

NetworkSettingsDialog::NetworkSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NetworkSettingsDialog)
    , model_(new NetSettingsModel(this))
    , manager_(0)
    , connectIndex_(-1)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->treeView->setModel(model_);
    loadSettings();
    createConnects();
}

NetworkSettingsDialog::~NetworkSettingsDialog()
{
    saveSettings();
    delete ui;
}

void NetworkSettingsDialog::setManager(SSHSettingsManager* manager)
{
    manager_ = manager;
    model_->setSettingsManager(manager_);
}

void NetworkSettingsDialog::createConnects()
{
    connect(ui->btnConnect, &QPushButton::clicked, this, [=](){
        int index = ui->treeView->currentIndex().row();
        if(index >= 0)
        {
            connectIndex_ = index;
            accept();
        }
    });

    connect(ui->btnNewConnect, &QPushButton::clicked, this, [=](){
        ConnectDialog dialog;
        dialog.setType(SSH);
        if(dialog.exec() == QDialog::Accepted)
        {
            SSHSettings::Ptr settings = dialog.sshSettings();
            manager_->addSettings(settings);
            model_->refresh();
        }
    });
    connect(ui->btnRename, &QPushButton::clicked, this, [=](){
        QModelIndex index = ui->treeView->currentIndex();
        QModelIndex nameIndex = model_->index(index.row(), 0);
        ui->treeView->edit(nameIndex);
    });
    connect(ui->btnEdit, &QPushButton::clicked, this, [=](){
        int index = ui->treeView->currentIndex().row();
        if(index >= 0)
        {
            ConnectDialog dialog;
            dialog.setType(SSH);
            dialog.setSshSettings(manager_->settings(index));
            if(dialog.exec() == QDialog::Accepted)
            {
                dialog.sshSettings();
                model_->refresh();
            }
        }
    });
    connect(ui->btnDelete, &QPushButton::clicked, this, [=](){
        int index = ui->treeView->currentIndex().row();
        if(index >= 0)
        {
            manager_->removeSettings(manager_->settings(index));
            model_->refresh();
        }
    });
}

void NetworkSettingsDialog::saveSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup("NetworkSettingsDialog");
    QHeaderView *headerView = ui->treeView->header();
    settings.beginWriteArray("sectionSizes", headerView->count());
    for(int i = 0; i < headerView->count(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("sectionSize", headerView->sectionSize(i));
    }
    settings.endArray();
    settings.endGroup();
}

void NetworkSettingsDialog::loadSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup("NetworkSettingsDialog");
    QHeaderView *headerView = ui->treeView->header();
    int size = settings.beginReadArray("sectionSizes");
    for(int i = 0; i < size && i < headerView->count(); i++)
    {
        settings.setArrayIndex(i);
        headerView->resizeSection(i, settings.value("sectionSize").toInt());
    }
    settings.endArray();
    settings.endGroup();
}
