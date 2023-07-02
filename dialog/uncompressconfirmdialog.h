#ifndef UNCOMPRESSCONFIRMDIALOG_H
#define UNCOMPRESSCONFIRMDIALOG_H
#include "core/fileuncompresser.h"
#include <QDialog>

namespace Ui {
class UnCompressConfirmDialog;
}

class UnCompressConfirmDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UnCompressConfirmDialog(QWidget *parent = nullptr);
    ~UnCompressConfirmDialog();

    void setTargetPath(QString const& filePath);
    QString targetPath() const;

    void setSettings(UncompressParam const& param);
    UncompressParam settings() const;
private:
    Ui::UnCompressConfirmDialog *ui;
    UncompressParam param_;
};

#endif // UNCOMPRESSCONFIRMDIALOG_H
