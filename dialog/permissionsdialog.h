#ifndef PERMISSIONSDIALOG_H
#define PERMISSIONSDIALOG_H

#include <QDialog>

namespace Ui {
class PermissionsDialog;
}

class PermissionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PermissionsDialog(QWidget *parent = nullptr);
    ~PermissionsDialog();

    void setFileName(QString const& fileName);
    void setPermissions(quint32 permissions, bool isDir);
    quint16 permissions() const;
    bool isRecursively() const;
private:
    void updatePermissions();
private:
    Ui::PermissionsDialog *ui;
    quint32 permissions_;
    bool isDir_;
};

#endif // PERMISSIONSDIALOG_H
