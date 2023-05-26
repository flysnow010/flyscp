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

    void setFileNames(QStringList const& fileNames);
    void setTargetFileName(QString const& fileName);
    QString targetFileName();

    bool isAlongWithPath() const;
    bool isRecursively() const;
    bool isCreateSFX() const;
    bool isCompressSignle() const;
    bool isEncryption() const;
    QString password() const;
private:
    void changeSuffix(QString const& suffix, bool isCanEncryption = false);
    void setLabel(int size);
private:
    Ui::CompressConfirmDialog *ui;
};

#endif // COMPRESSCONFIRMDIALOG_H
