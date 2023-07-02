#include "netsettingsmodel.h"
#include "core/sshsettings.h"

NetSettingsModel::NetSettingsModel(QObject *parent)
    : TreeModel(parent)
    , settingsManager(0)
{
    setupData();
}

void NetSettingsModel::setSettingsManager(SSHSettingsManager* manager)
{
    settingsManager = manager;
    setupData();
}

TreeItem *NetSettingsModel::createRootItem()
{
    QList<QVariant> rootData;
    rootData << tr("Name") << tr("User") << tr("Host") << tr("Port");
    TreeItem* item = new TreeItem(rootData);
    return item;
}

void NetSettingsModel::refresh()
{
    setupData();
}

bool NetSettingsModel::setData(const QModelIndex &index,
                               const QVariant &value, int role)
{
    if(index.column() == 0 && role == Qt::EditRole)
    {
        QString newName = value.toString();
        if(!newName.isEmpty())
         {
            SSHSettings::Ptr settings = settingsManager->settings(index.row());
            settings->name = newName;
            TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
            return item->setData(index.column(), value);
        }
    }
    return false;
}

bool NetSettingsModel::editable(const QModelIndex &index) const
{
    if(index.column() == 0)
        return true;
    return false;
}

void NetSettingsModel::setupModelData(TreeItem *parent)
{
    if(!settingsManager)
        return;

    for(int i = 0; i < settingsManager->size(); i++)
    {
        QList<QVariant> rowData;
        SSHSettings::Ptr settings = settingsManager->settings(i);
        rowData << settings->name
                << settings->userName
                << settings->hostName
                << settings->port;
        TreeItem* item = new TreeItem(rowData, parent);
        parent->appendChild(item);
    }
}
