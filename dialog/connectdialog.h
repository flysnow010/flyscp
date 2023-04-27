#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>
#include "core/connecttype.h"
#include "core/sshsettings.h"

namespace Ui {
class ConnectDialog;
}

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(QWidget *parent = nullptr);
    ~ConnectDialog();

    ConnectType type() const;
    void setType(ConnectType t);

    SSHSettings sshSettings() const;
    void setSshSettings(SSHSettings const& settings);

private slots:
    void selectPrivateKeyFileName();
private:

private:
    Ui::ConnectDialog *ui;
};

#endif // CONNECTDIALOG_H
