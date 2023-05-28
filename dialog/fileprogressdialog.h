#ifndef FILEPROGRESSDIALOG_H
#define FILEPROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class FileProgressDialog;
}

class FileProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileProgressDialog(QWidget *parent = nullptr);
    ~FileProgressDialog();

    bool isFinished() const { return isFinished_; }
    bool isCancel() const { return isCancel_; }

    void setStatusTextMode();
public slots:
    void totalProgress(QString const& srcFilename,
                       QString const& dstFilename,
                       int totalSize, int totalSizeTransferred);
    void fileProgress(qint64 totalFileSize, qint64 totalBytesTransferred);
    void progressText(QString const& text);
    void finished();
    void error(QString const& e);
private:
    Ui::FileProgressDialog *ui;
    bool isFinished_;
    bool isCancel_;
};

#endif // FILEPROGRESSDIALOG_H
