#include "fileprogressdialog.h"
#include "ui_fileprogressdialog.h"

FileProgressDialog::FileProgressDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FileProgressDialog)
    , isFinished_(false)
    , isCancel_(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    connect(ui->cancelButton, &QPushButton::clicked, [=](){
        isCancel_= true;
    });
}

FileProgressDialog::~FileProgressDialog()
{
    delete ui;
}

void FileProgressDialog::totalProgress(QString const& srcFilename,
                   QString const& dstFilename,
                   int totalSize, int totalSizeTransferred)
{
    ui->srcFileName->setText(srcFilename);
    ui->dstFileName->setText(dstFilename);
    if(totalSize > 0)
        ui->totalProgressBar->setValue(totalSizeTransferred * 100 / totalSize);
}

void FileProgressDialog::fileProgress(qint64 totalFileSize, qint64 totalBytesTransferred)
{
    if(totalFileSize > 0)
        ui->fileProgressBar->setValue(totalBytesTransferred * 100 / totalFileSize);
}

void FileProgressDialog::finished()
{
    isFinished_ = true;
}

void FileProgressDialog::error(QString const&)
{

}
