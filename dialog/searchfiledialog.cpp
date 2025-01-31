#include "searchfiledialog.h"
#include "ui_searchfiledialog.h"
#include "util/utils.h"

#include "core/filetransfer.h"
#include "core/sftpfilemanager.h"
#include "core/remotefiletransfer.h"
#include "model/stringlistmodel.h"
#include <QApplication>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QMenu>
#include <QSettings>
#include <QDebug>

SearchFileDialog::SearchFileDialog(Mode mode, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SearchFileDialog)
    , mode_(mode)
    , dirverMenu(new QMenu(this))
    , sftpSession(0)
    , model(new StringListModel(this))
    , isSearching(false)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Widget);
    startSearch(false);
    if(mode_ == Local)
    {
        ui->btnDrivers->setMenu(dirverMenu);
        createDriverMenu();
    }
    else
    {
        ui->btnBrowseFolder->hide();
        ui->btnDrivers->hide();
        ui->gridLayout->setHorizontalSpacing(0);
    }

    connect(ui->lwResult, &QListView::doubleClicked,
            this, [=](QModelIndex const& index)
    {
        if(index.isValid() && index.row() > 0)
        {
            emit goToFile(fileNames[index.row()]);
            accept();
        }
    });

    connect(ui->btnView, &QPushButton::clicked,
            this, [=](bool)
    {
        QModelIndex index = ui->lwResult->currentIndex();
        if(index.isValid() && index.row() > 0)
            emit viewFile(fileNames[index.row()]);
    });

    connect(ui->btnNew, &QPushButton::clicked,
            this, [=](bool){ startSearch(false); });

    connect(ui->btnGoToFile, &QPushButton::clicked,
            this, [=](bool)
    {
        QModelIndex index = ui->lwResult->currentIndex();
        if(index.isValid() && index.row() > 0)
        {
            emit goToFile(fileNames[index.row()]);
            accept();
        }
    });

    connect(ui->btnCancel, &QPushButton::clicked,
            this, [=](bool)
    {
        if(isSearching)
            isSearching = false;
        else
            reject();
    });

    connect(ui->btnBrowseFolder, &QToolButton::clicked,
            this, [=]
    {
        QString filePath = Utils::getPath(tr("Open Directory"));
        if(!filePath.isEmpty())
            ui->cbFolder->setCurrentText(filePath);
    });

    connect(ui->btnSave, &QPushButton::clicked,
            this, [=]
    {
        QString fileName =  Utils::getSaveFile(tr("Save as"));
        if(!fileName.isEmpty())
        {
            Utils::saveFile(fileName, fileNames.join("\n"));
        }
    });

    connect(ui->btnStartSearch, &QPushButton::clicked,
            this, &SearchFileDialog::searchFiles);

    loadSettings();
}

SearchFileDialog::~SearchFileDialog()
{
    saveSettings();
    delete ui;
}

void SearchFileDialog::setSearchPath(QString  const& filePath)
{
    ui->cbFolder->setCurrentText(filePath);
}

void SearchFileDialog::setSfpSession(SFtpSession* session)
{
    sftpSession = session;
}

void SearchFileDialog::searchFiles()
{
    startSearch(true);
    fileNames.clear();
    ui->labelCurentPath->setText("");
    model->setStringList(fileNames);
    ui->lwResult->setModel(model);

    bool isFinished = false;
    int fileCount = 0;
    int dirCount = 0;
    int index = 0;
    isSearching = true;
    setSeearchState(isSearching);
    if(mode_ == Local)
    {
        FileTransfer fileSearcher;

        connect(&fileSearcher, &FileTransfer::currentFolder,
                this, [&](QString const& filePath)
        {
            ui->labelCurentPath->setText(filePath);
        });

        connect(&fileSearcher, &FileTransfer::foundFile,
                this, [&](QString const& fileName)
        {
            fileNames << fileName;
            fileCount++;
        });

        connect(&fileSearcher, &FileTransfer::foundFolder,
                this, [&](QString const& filePath)
        {
            fileNames << filePath;
            dirCount++;
        });

        connect(&fileSearcher, &FileTransfer::finished,
                this, [&](){ isFinished = true; });

        fileSearcher.searchFiles(ui->cbFolder->currentText(),
                                 ui->cbFileName->currentText());
        addCurentItem(ui->cbFileName);
        addCurentItem(ui->cbFolder);
        while(!isFinished)
        {
            if(!isSearching)
                fileSearcher.cancel();
            if(index < fileNames.size())
            {
                model->insertRow(index, fileNames[index]);
                index++;
            }
            QApplication::processEvents();
        }
    }
    else
    {
        RemoteFileTransfer fileSearcher(new SFtpFileManager(sftpSession));
        connect(&fileSearcher, &RemoteFileTransfer::currentFolder,
                this, [&](QString const& filePath)
        {
            ui->labelCurentPath->setText(filePath);
        });

        connect(&fileSearcher, &RemoteFileTransfer::foundFile,
                this, [&](QString const& fileName)
        {
            fileNames << fileName;
            fileCount++;
        });

        connect(&fileSearcher, &RemoteFileTransfer::foundFolder,
                this, [&](QString const& filePath)
        {
            fileNames << filePath;
            dirCount++;
        });

        connect(&fileSearcher, &RemoteFileTransfer::finished,
                this, [&](){ isFinished = true; });

        fileSearcher.searchFiles(ui->cbFolder->currentText(),
                                 ui->cbFileName->currentText());
        addCurentItem(ui->cbFileName);
        addCurentItem(ui->cbFolder);
        while(!isFinished)
        {
            if(!isSearching)
                fileSearcher.cancel();
            if(index < fileNames.size())
            {
                model->insertRow(index, fileNames[index]);
                index++;
            }
            QApplication::processEvents();
        }
    }

    QString result = QString(tr("Found %1 files, %2 folders"))
            .arg(fileCount).arg(dirCount);
    if(!isSearching)
    {
        result += tr(" - Search is stoped");
        setSeearchState(isSearching);
    }
    ui->labelCurentPath->setText(result);

    if(isSearching)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        model->appendRows(index, fileNames);
        fileNames.push_front(result);
        insertText(0, result);
        QApplication::restoreOverrideCursor();
        isSearching = false;
        setSeearchState(isSearching);
    }
}

void SearchFileDialog::setSeearchState(bool isSearching)
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

void SearchFileDialog::startSearch(bool isStart)
{
    ui->widgetResult->setVisible(isStart);
    ui->widgetButtons->setVisible(isStart);
    ui->frameCurrentDir->setVisible(isStart);
    adjustSize();
}

void SearchFileDialog::insertText(int row, QString const& text)
{
    model->insertRows(row, 1);
    model->setData(model->index(row), text);
}

void SearchFileDialog::createDriverMenu()
{
    QFileInfoList fileInfos = QDir::drives();
    QFileIconProvider iconProvider;

    foreach(auto fileInfo, fileInfos)
    {
        dirverMenu->addAction(iconProvider.icon(fileInfo),
                              fileInfo.filePath().at(0),
                              this, [=](){
            ui->cbFolder->setCurrentText(fileInfo.filePath());
        });
    }
}

#define MAX_ITEM_COUNT 10
void SearchFileDialog::addCurentItem(QComboBox* cb)
{
    QString text = cb->currentText();
    int index = cb->findText(text);

    if(index < 0)
        cb->insertItem(0, text);
    else if(index > 0)
    {
        cb->removeItem(index);
        cb->insertItem(0, text);
    }

    if(cb->count() > MAX_ITEM_COUNT)
        cb->removeItem(cb->count() - 1);
    cb->setCurrentText(text);
}

void SearchFileDialog::saveSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup("SerchFileDialog");
    if(mode_ == Local)
        settings.beginGroup("Local");
    else
        settings.beginGroup("Remote");
    settings.setValue("currentFileName", ui->cbFileName->currentText());
    settings.setValue("currentFilePath", ui->cbFolder->currentText());

    settings.beginWriteArray("fileNames", ui->cbFileName->count());
    for(int i = 0; i < ui->cbFileName->count(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("fileName", ui->cbFileName->itemText(i));
    }
    settings.endArray();

    settings.beginWriteArray("filePaths", ui->cbFolder->count());
    for(int i = 0; i < ui->cbFolder->count(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("filePath", ui->cbFolder->itemText(i));
    }
    settings.endArray();
    settings.endGroup();

    settings.endGroup();
}

void SearchFileDialog::loadSettings()
{
    QSettings settings(QCoreApplication::applicationName(),
                       QCoreApplication::applicationVersion());
    settings.beginGroup("SerchFileDialog");

    if(mode_ == Local)
        settings.beginGroup("Local");
    else
        settings.beginGroup("Remote");
    int size = settings.beginReadArray("fileNames");
    for(int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        ui->cbFileName->addItem(settings.value("fileName").toString());
    }
    settings.endArray();

    size = settings.beginReadArray("filePaths");
    for(int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        ui->cbFolder->addItem(settings.value("filePath").toString());
    }
    settings.endArray();

    settings.endGroup();

    QString currentFileName = settings.value("currentFileName").toString();
    QString currentFilePath = settings.value("currentFilePath").toString();

    if(currentFileName.isEmpty())
        currentFileName = "*";
    ui->cbFileName->setCurrentText(currentFileName);
    ui->cbFolder->setCurrentText(currentFilePath);

    settings.endGroup();
}
