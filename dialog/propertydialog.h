#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H
#include "ssh/fileinfo.h"
#include <QDialog>

namespace Ui {
class PropertyDialog;
}

class PropertyDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PropertyDialog(QWidget *parent = nullptr);
    ~PropertyDialog();

    void setFileInfo(ssh::FileInfoPtr const& fileInfo);

private:
    Ui::PropertyDialog *ui;
};

#endif // PROPERTYDIALOG_H
