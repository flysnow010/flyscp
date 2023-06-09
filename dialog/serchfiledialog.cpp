#include "serchfiledialog.h"
#include "ui_serchfiledialog.h"
#include "util/utils.h"

#include "core/filetransfer.h"
#include <QApplication>
#include <QStringListModel>
#include <QDebug>

SerchFileDialog::SerchFileDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SerchFileDialog)
    , model(new QStringListModel(this))
    , isSearching(false)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);
    startSearch(false);


    connect(ui->btnView, &QPushButton::clicked, this, [=](bool){
        QModelIndex index = ui->lwResult->currentIndex();
        if(index.isValid() && index.row() > 0)
            emit viewFile(fileNames[index.row()]);
    });

    connect(ui->btnNew, &QPushButton::clicked, this, [=](bool){
        startSearch(false);
    });

    connect(ui->btnGoToFile, &QPushButton::clicked, this, [=](bool){
        QModelIndex index = ui->lwResult->currentIndex();
        if(index.isValid() && index.row() > 0)
        {
            emit goToFile(fileNames[index.row()]);
            accept();
        }
    });

    connect(ui->btnCancel, &QPushButton::clicked, this, [=](bool){
        if(isSearching)
            isSearching = false;
        else
            reject();
    });

    connect(ui->btnBrowseFolder, &QToolButton::clicked, this, [=]{
        QString filePath = Utils::getPath("Open Directory");
        if(!filePath.isEmpty())
            ui->cbFolder->setCurrentText(filePath);
    });

    connect(ui->btnSave, &QPushButton::clicked, this, [=]{
        QString fileName =  Utils::getSaveFile("Save as");
        if(!fileName.isEmpty())
        {
            Utils::saveFile(fileName, fileNames.join("\n"));
        }
    });

    connect(ui->btnStartSearch, &QPushButton::clicked, this, &SerchFileDialog::searchFiles);
}

SerchFileDialog::~SerchFileDialog()
{
    delete ui;
}

void SerchFileDialog::setSearchPath(QString  const& filePath)
{
    ui->cbFolder->setCurrentText(filePath);
}

void SerchFileDialog::searchFiles()
{
    startSearch(true);
    fileNames.clear();
    ui->labelCurentPath->setText("");
    model->setStringList(fileNames);
    ui->lwResult->setModel(model);

    FileTransfer fileSearcher;
    bool isFinished = false;
    int fileCount = 0;
    int dirCount = 0;
    int index = 0;
    isSearching = true;

    setSeearchState(isSearching);

    connect(&fileSearcher, &FileTransfer::currentFolder, this, [&](QString const& filePath){
        ui->labelCurentPath->setText(filePath);
    });
    connect(&fileSearcher, &FileTransfer::foundFile, this, [&](QString const& fileName){
        fileNames << fileName;
        fileCount++;
    });
    connect(&fileSearcher, &FileTransfer::foundFolder, this, [&](QString const& filePath){
        fileNames << filePath;
        dirCount++;
    });
    connect(&fileSearcher, &FileTransfer::finished, this, [&](){
        isFinished = true;
    });

    fileSearcher.searchFiles(ui->cbFolder->currentText(), ui->cbFileName->currentText());
    while(!isFinished)
    {
        if(!isSearching)
            fileSearcher.cancel();
        if(index < fileNames.size())
        {
            insertText(index, fileNames[index]);
            index++;
        }
        QApplication::processEvents();
    }

    QString result = QString("Found %1 files, %2 folders").arg(fileCount).arg(dirCount);
    if(!isSearching)
        result += " - Search is stoped";
    ui->labelCurentPath->setText(result);
    qDebug() << index << fileNames.size();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    while(index < fileNames.size())
    {
        insertText(index, fileNames[index]);
        index++;
    }
    fileNames.push_front(result);
    insertText(index, result);
    QApplication::restoreOverrideCursor();
    isSearching = false;
    setSeearchState(isSearching);
}

void SerchFileDialog::setSeearchState(bool isSearching)
{
    ui->btnBrowseFolder->setDisabled(isSearching);
    ui->btnDrivers->setDisabled(isSearching);
    ui->btnStartSearch->setDisabled(isSearching);
    ui->btnView->setDisabled(isSearching);
    ui->btnNew->setDisabled(isSearching);
    ui->btnGoToFile->setDisabled(isSearching);
    ui->btnSave->setDisabled(isSearching);
    ui->btnStartSearch->setDisabled(isSearching);
}

void SerchFileDialog::startSearch(bool isStart)
{
    ui->widgetResult->setVisible(isStart);
    ui->widgetButtons->setVisible(isStart);
    ui->frameCurrentDir->setVisible(isStart);
    adjustSize();
}

void SerchFileDialog::insertText(int row,  QString const& text)
{
    model->insertRows(row, 1);
    model->setData(model->index(row), text);
}
