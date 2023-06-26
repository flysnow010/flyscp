#include "compressdirmodel.h"
#include "core/fileuncompresser.h"
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

void CompressDirModel::setCompressFile(QFileInfo const& fileInfo)
{
    QStringList fileInfoLines = FileUncompresser().listFileInfo(fileInfo.filePath());
    compressFile.setFileName(fileInfo.fileName(), fileInfoLines);
    fileInfos_ = compressFile.fileInfoList(CompressFile::DirsFirst);
    setupData();
}

CompressFileInfo::Ptr CompressDirModel::fileInfo(int index)
{
    return fileInfos_.at(index);
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
