#ifndef NETWORKSETTINGSDIALOG_H
#define NETWORKSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class NetworkSettingsDialog;
}

class NetworkSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkSettingsDialog(QWidget *parent = nullptr);
    ~NetworkSettingsDialog();

private:
    Ui::NetworkSettingsDialog *ui;
};

#endif // NETWORKSETTINGSDIALOG_H
