#include "compressdirmodel.h"
#include "util/utils.h"
#include <QFileInfo>
namespace  {
int const NAME_INDEX = 0;
int const SIZE_INDEX = 2;
}

CompressDirModel::CompressDirModel(QObject *parent)
    : DirModel(parent)
    , dirIcon(Utils::dirIcon())
    , fileIcon(Utils::fileIcon())
    , backIcon(":/image/back.png")
{
    setupData();
}

TreeItem* CompressDirModel::createRootItem()
{
    QList<QVariant> rootData;
    rootData << tr("Name") << tr("Suffix") << tr("Size") << tr("Date Modified");
    TreeItem* item = new TreeItem(rootData);
    return item;
}

QVariant CompressDirModel::textAlignment(const QModelIndex &index) const
{
    if(index.column() == NAME_INDEX)
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    else if(index.column() == SIZE_INDEX)
    {
        if(fileInfos_[index.row()]->isFile())
            return int(Qt::AlignRight | Qt::AlignVCenter);
    }
    return QVariant();
}

bool CompressDirModel::editable(const QModelIndex &index) const
{
    if(index.column() == 0 && !fileInfos_[index.row()]->isParent())
        return true;
    return false;
}

QVariant CompressDirModel::editText(const QModelIndex &index) const
{
    if(index.column() == 0)
    {
        if(!isRenameBaseName() || fileInfos_[index.row()]->baseName().isEmpty())
            return fileInfos_[index.row()]->fileName();
        else
            return fileInfos_[index.row()]->baseName();
    }
    return QVariant();
}

bool CompressDirModel::cd(QString const& dir)
{
    if(!compressFile.cd(dir))
        return false;
    fileInfos_ = compressFile.fileInfoList(CompressFile::DirsFirst);
    setupData();
    return true;
}

QVariant CompressDirModel::icon(const QModelIndex &index) const
{
    if(index.column() != 0)
        return QVariant();

    if(fileInfos_[index.row()]->isDir())
        return dirIcon;
    return fileIcon;
}

QString CompressDirModel::dir() const
{
    return compressFile.dir();
}

void CompressDirModel::setCompressFile(QFileInfo const& fileInfo)
{
    compressFile.setFileName(fileInfo.filePath());
    fileInfos_ = compressFile.fileInfoList(CompressFile::DirsFirst);
    setupData();
}

bool CompressDirModel::setDir(QString const& dir)
{
    if(compressFile.setDir(dir))
    {
        fileInfos_ = compressFile.fileInfoList(CompressFile::DirsFirst);
        setupData();
        return true;
    }
    return false;
}

void CompressDirModel::refresh()
{
    compressFile.refresh();
    fileInfos_ = compressFile.fileInfoList(CompressFile::DirsFirst);
    setupData();
}

bool CompressDirModel::rmFile(QString const& filePath)
{
    return compressFile.rmFile(filePath);
}

bool CompressDirModel::rename(QString const& oldFileName, QString const& newFileName)
{
    return compressFile.rename(oldFileName, newFileName);
}

bool CompressDirModel::extract(QString const& targetPath,
                               QString const& filePath,
                               bool isWithPath)
{
    return compressFile.extract(targetPath, filePath, isWithPath);
}

CompressFileInfo::Ptr CompressDirModel::fileInfo(int index)
{
    return fileInfos_.at(index);
}

bool CompressDirModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.column() == 0 && role == Qt::EditRole)
    {
        QString newName = value.toString();
        if(!newName.isEmpty())
         {
            CompressFileInfo::Ptr fileInfo = fileInfos_.at(index.row());
            QString oldFileName = fileInfo->fileName();
            QString suffix = fileInfo->suffix();
            QString newFileName;
            if(!isRenameBaseName() || suffix.isEmpty())
                newFileName = newName;
            else
               newFileName = QString("%1.%2").arg(newName, suffix);
            if(oldFileName == newFileName)
                return false;
            if(rename(oldFileName, newFileName))
            {
                TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
                return item->setData(index.column(), value);
            }
        }
    }
    return false;
}

void CompressDirModel::setupModelData(TreeItem *parent)
{
    for(int i = 0; i < fileInfos_.size();i++)
    {
        QList<QVariant> rowData;
        if(fileInfos_[i]->isDir())
            rowData << fileInfos_[i]->path() << QString() << QString("<DIR>");
        else
            rowData << fileInfos_[i]->fileName() << fileInfos_[i]->suffix()
                    << Utils::formatFileSizeB(fileInfos_[i]->size());
        rowData << fileInfos_[i]->timeText();
        TreeItem* item = new TreeItem(rowData, parent);
        parent->appendChild(item);
    }
}
