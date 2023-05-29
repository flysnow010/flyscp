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

    QString overwriteMode() const;
};

class QStringList;
class QProcess;

class FileUncompresser: public QObject
{
    Q_OBJECT
public:
    explicit FileUncompresser(QObject *parent = nullptr);

    bool uncompress(QStringList const& fileNames,
                    UncompressParam const& param,
                    QString const& targetFilePath);
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
};

#endif // FILEUNCOMPRESSER_H
