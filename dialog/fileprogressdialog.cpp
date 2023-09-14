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

    connect(ui->cancelButton, &QPushButton::clicked, this, [=](){
        isCancel_= true;
    });
}

FileProgressDialog::~FileProgressDialog()
{
    delete ui;
}

void FileProgressDialog::hideTotalProgress()
{
    ui->totalProgressBar->hide();
    ui->totalLabel->hide();
    adjustSize();
}

void FileProgressDialog::setStatusTextMode()
{
    ui->label1->hide();
    ui->label2->hide();
    ui->totalLabel->hide();
    ui->label4->hide();
    ui->totalProgressBar->hide();
    ui->fileProgressBar->hide();
    adjustSize();
}

void FileProgressDialog::totalProgress(QString const& srcFilename,
                                       QString const& dstFilename,
                                       int totalSize,
                                       int totalSizeTransferred)
{
    ui->srcFileName->setText(srcFilename);
    ui->dstFileName->setText(dstFilename);
    if(totalSize > 0)
        ui->totalProgressBar->setValue(totalSizeTransferred * 100 / totalSize);
}

void FileProgressDialog::fileProgress(qint64 totalFileSize,
                                      qint64 totalBytesTransferred)
{
    if(totalFileSize > 0)
        ui->fileProgressBar->setValue(totalBytesTransferred * 100 / totalFileSize);
}

void FileProgressDialog::progressText(QString const& text)
{
    ui->srcFileName->setText(text);
}

void FileProgressDialog::finished()
{
    isFinished_ = true;
}

void FileProgressDialog::error(QString const&)
{
}
