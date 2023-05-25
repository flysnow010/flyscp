#ifndef UNCOMPRESSCONFIRMDIALOG_H
#define UNCOMPRESSCONFIRMDIALOG_H

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
    QString targetFileName() const;
    bool isAlongWithPath() const;
    bool isOverwrite() const;
private:
    Ui::UnCompressConfirmDialog *ui;
};

#endif // UNCOMPRESSCONFIRMDIALOG_H
