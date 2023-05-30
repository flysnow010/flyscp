#ifndef COMPRESSCONFIRMDIALOG_H
#define COMPRESSCONFIRMDIALOG_H
#include "core/filecompresser.h"

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

    void setSettings(CompressParam const& param);
    CompressParam settings() const;
    static QString CurrentSuffix();
    static void setCurrentSuffix(QString const& suffix);
private:
    void changeSuffix(QString const& suffix, bool isCanEncryption = false);
    void setLabel(int size);
private:
    Ui::CompressConfirmDialog *ui;
    QString suffix_;
    static QString currentSuffix;
};

#endif // COMPRESSCONFIRMDIALOG_H
