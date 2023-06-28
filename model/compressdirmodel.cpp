#include "compressdirmodel.h"
#include "util/utils.h"
#include <QFileInfo>
namespace  {
int const NONE_INDEX = -1;
int const NAME_INDEX = 0;
int const SUFFIX_INDEX = 1;
int const SIZE_INDEX = 2;
int const TIME_INDEX = 3;
}

CompressDirModel::CompressDirModel(QObject *parent)
    : DirModel(parent)
    , dirIcon(Utils::dirIcon())
    , fileIcon(Utils::fileIcon())
    , backIcon(":/image/back.png")
    , sortIndex(NONE_INDEX)
    , isDescending(false)
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

void CompressDirModel::sortItems(int index, bool isDescendingOrder)
{
     uint32_t sortFlag = CompressFile::DirsFirst;

    if(index == NAME_INDEX)
    {
        if(dirSortIsByTime())
            sortFlag |=  CompressFile::Time;
        else
            sortFlag |= CompressFile::Name;
    }
    else if(index == SIZE_INDEX)
        sortFlag |=  CompressFile::Size;
    else if(index == TIME_INDEX)
        sortFlag |=  CompressFile::Time;
    else if(index == SUFFIX_INDEX)
        sortFlag |= CompressFile::Type;
    if(!isDescendingOrder)
        sortFlag |=  CompressFile::Reversed;
    fileInfos_ = compressFile.fileInfoList( static_cast<CompressFile::SortFlag>(sortFlag));
    sortIndex = index;
    isDescending = isDescendingOrder;
    setupData();
}

QVariant CompressDirModel::icon(const QModelIndex &index) const
{
    if(index.column() != 0)
        return QVariant();

    if(fileInfos_[index.row()]->isParent())
        return backIcon;

    if(fileInfos_[index.row()]->isDir())
        return dirIcon;

    return fileIcon;
}

QString CompressDirModel::dir() const
{
    return compressFile.dir();
}

void CompressDirModel::setCompressFileName(QString const& fileName)
{
    compressFile.setFileName(fileName);
    fileInfos_ = compressFile.fileInfoList(CompressFile::DirsFirst);
    setupData();
}

QString CompressDirModel::compressFileName() const
{
    return compressFile.fileName();
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
    if(sortIndex != NONE_INDEX)
        sortItems(sortIndex, isDescending);
    else
    {
        fileInfos_ = compressFile.fileInfoList(CompressFile::DirsFirst);
        setupData();
    }
}

bool CompressDirModel::rm(QString const& filePath)
{
    return compressFile.rm(QStringList() << filePath);
}

bool CompressDirModel::rm(QStringList const& fileNames)
{
    return compressFile.rm(fileNames);
}

bool CompressDirModel::rename(QString const& oldFileName, QString const& newFileName)
{
    return compressFile.rename(oldFileName, newFileName);
}

bool CompressDirModel::extract(QString const& targetPath,
                               QStringList const& fileNames,
                               bool isWithPath)
{
    return compressFile.extract(targetPath, fileNames, isWithPath);
}

CompressFileInfo::Ptr CompressDirModel::fileInfo(int index)
{
    return fileInfos_.at(index);
}

bool CompressDirModel::isParent(int index) const
{
    return fileInfos_.at(index)->isParent();
}

bool CompressDirModel::isDir(int index) const
{
    return fileInfos_.at(index)->isDir();
}

bool CompressDirModel::isFile(int index) const
{
    return fileInfos_.at(index)->isFile();
}

qint64 CompressDirModel::fileSize(int index) const
{
    return fileInfos_.at(index)->size();
}

QString CompressDirModel::filePath(int index) const
{
    return fileInfos_.at(index)->filePath();
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
    fileCount_ = 0;
    dirCount_ = 0;
    fileSizes_ = 0;
    for(int i = 0; i < fileInfos_.size();i++)
    {
        QList<QVariant> rowData;
        if(fileInfos_[i]->isDir())
        {
            rowData << fileInfos_[i]->path() << QString() << QString("<DIR>");
            if(!fileInfos_[i]->isParent())
                dirCount_++;
        }
        else
        {
            rowData << fileInfos_[i]->baseName() << fileInfos_[i]->suffix()
                    << Utils::formatFileSizeB(fileInfos_[i]->size());
            fileCount_++;
            fileSizes_ += fileInfos_[i]->size();
        }
        rowData << fileInfos_[i]->timeText();
        TreeItem* item = new TreeItem(rowData, parent);
        parent->appendChild(item);
    }
}
