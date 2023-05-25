#ifndef COMPRESSCONFIRMDIALOG_H
#define COMPRESSCONFIRMDIALOG_H

#include <QDialog>

namespace Ui {
class CompressConfirmDialog;
}

class CompressConfirmDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompressConfirmDialog(QWidget *parent = nullptr);
    ~CompressConfirmDialog();

    void setFileCount(int size);
    void setTargetFileName(QString const& fileName);
    QString targetFileName();
private:
    Ui::CompressConfirmDialog *ui;
};

#endif // COMPRESSCONFIRMDIALOG_H
