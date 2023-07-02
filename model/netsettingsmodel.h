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
                 const QVariant &value, int role);
protected:
    virtual TreeItem *createRootItem();
    virtual void setupModelData(TreeItem *parent);
    virtual bool editable(const QModelIndex &index) const;
private:
    SSHSettingsManager* settingsManager;
};

#endif // NETSETTINGSMODEL_H
