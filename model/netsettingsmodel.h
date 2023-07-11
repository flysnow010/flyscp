#ifndef NETSETTINGSMODEL_H
#define NETSETTINGSMODEL_H

#include "treemodel.h"

class SSHSettingsManager;
class NetSettingsModel : public TreeModel
{
    Q_OBJECT
public:
    explicit NetSettingsModel(QObject *parent = nullptr);

    void setSettingsManager(SSHSettingsManager* manager);
    void refresh();

    bool setData(const QModelIndex &index,
                 const QVariant &value, int role) override;
protected:
    TreeItem *createRootItem() override;
    void setupModelData(TreeItem *parent) override;
    bool editable(const QModelIndex &index) const override;
private:
    SSHSettingsManager* settingsManager;
};

#endif // NETSETTINGSMODEL_H
