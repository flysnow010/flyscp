#ifndef NETWORKSETTINGSDIALOG_H
#define NETWORKSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class NetworkSettingsDialog;
}
class NetSettingsModel;
class SSHSettingsManager;
class NetworkSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkSettingsDialog(QWidget *parent = nullptr);
    ~NetworkSettingsDialog();

    void setManager(SSHSettingsManager* manager);
    int connectIndex() const { return connectIndex_; }
private:
    void createConnects();
    void saveSettings();
    void loadSettings();
private:
    Ui::NetworkSettingsDialog *ui;
    NetSettingsModel* model_;
    SSHSettingsManager* manager_;
    int connectIndex_;
};

#endif // NETWORKSETTINGSDIALOG_H
