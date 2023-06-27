#ifndef FILEOPERATECONFIRMDIALOG_H
#define FILEOPERATECONFIRMDIALOG_H

#include <QDialog>

namespace Ui {
class FileOperateConfirmDialog;
}

class FileOperateConfirmDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileOperateConfirmDialog(QWidget *parent = nullptr);
    ~FileOperateConfirmDialog();

    void setLabel(QString const& label);
    void setPath(QString const& path);
    QString path() const;
    QString fileType() const;

    bool isToQueue()  const { return isToQueue_; }
private:
    Ui::FileOperateConfirmDialog *ui;
    bool isToQueue_;
};

#endif // FILEOPERATECONFIRMDIALOG_H
