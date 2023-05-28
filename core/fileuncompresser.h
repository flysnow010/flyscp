#ifndef FILEUNCOMPRESSER_H
#define FILEUNCOMPRESSER_H
#include <QString>
#include <QObject>

class QStringList;
struct UncompressParam
{
    enum OverwriteMode{ OverWrite, Skip, AutoRename };

    bool isWithPath = true;
    bool isCreateDir = false;
    OverwriteMode mode = AutoRename;
    QString filter = QString("*");

    QString overwriteMode() const;
};

class FileUncompresser: public QObject
{
    Q_OBJECT
public:
    explicit FileUncompresser(QObject *parent = nullptr);

    bool uncompress(QStringList const& fileNames,
                    UncompressParam const& param,
                    QString const& targetFilePath);

signals:
    void progress(QString const& text);
    void error(QString const& error);
    void finished();
};

#endif // FILEUNCOMPRESSER_H
