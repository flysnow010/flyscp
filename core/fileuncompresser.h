#ifndef FILEUNCOMPRESSER_H
#define FILEUNCOMPRESSER_H
#include <QString>
#include <QObject>


struct UncompressParam
{
    enum OverwriteMode{ OverWrite, Skip, AutoRename };

    bool isWithPath = true;
    bool isCreateDir = false;
    OverwriteMode mode = AutoRename;
    QString filter = QString("*");
    QString password;

    QString overwriteMode() const;
};

class QStringList;
class QProcess;

class FileUncompresser: public QObject
{
    Q_OBJECT
public:
    explicit FileUncompresser(QObject *parent = nullptr);

    enum Mode { Uncompress, CheckEncrypt, List, Delete, Extract, Rename };

    static bool isCompressFiles(QStringList const& fileNames, QString &unCompressfileName);
    bool uncompress(QStringList const& fileNames,
                    UncompressParam const& param,
                    QString const& targetFilePath);

    bool isEncrypted(QString const& fileName);
    QStringList listFileInfo(QString const& fileName);
    bool remove(QString const& archiveFileName, QStringList const& fileNames);
    bool rename(QString const& archiveFileName, QString const& oldName, QString const& newName);
    bool rename(QString const& archiveFileName, QStringList const& fileNames);
    bool extract(QString const& archiveFileName,
                 QString const& targetPath,
                 QStringList const& fileNames, bool isWithPath);

    void cancel();
signals:
    void progress(QString const& text);
    void error(QString const& error);
    void finished();
private:
    void onError(QString const& error);
    QString errorToText(int errorCode) const;
    QStringList nextArgs() const;
private:
    QProcess* process;
    int currentIndex;
    QList<QStringList> argsList;
    QStringList fileInfos;
    Mode mode;
    bool isEncrypted_;
    bool isListStart_;
    bool isOK_;
};

#endif // FILEUNCOMPRESSER_H
