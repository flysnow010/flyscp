#ifndef FILENAME_H
#define FILENAME_H
#include <QString>
#include <QList>

struct FileName
{
    QString src;
    QString dst;
};

class FileNames : public QList<FileName>
{
public:
    static FileNames GetFileNames(QString const& fileName,
                                  QString const& filePath, QString const& prefix = QString());
    static FileNames GetFileNames(QStringList const& fileNames,
                                  QString const& filePath, QString const& prefix = QString());
    static void MakeFileNames(FileNames const& fileNames, FileNames & newFileNames);
    static void MakeFileNames(QStringList const& fileNames, QStringList & newFileNames);
    static void FindFilenames(FileName const& fileName, FileNames &fileNames);
    static void FindFilenames(QString const& fileName, QStringList &fileNames);
};
#endif
