#ifndef FILECOMPRESSER_H
#define FILECOMPRESSER_H
#include <QString>
#include <QObject>

struct CompressParam
{
    QString suffix = QString(".zip");
    QString password;
    QString filter = QString("*");
    unsigned int volumeSize = 0;
    bool isWithPath = true;
    bool isRecursively = true;
    bool isMultiVolume = false;
    bool isMoveFile = false;
    bool isCreateSFX = false;
    bool isSignle = false;
    bool isEncryption = false;
    QString volumeText() const;
    QString passwordText() const;
    bool isMultiSuffix() const;
};

class QStringList;
class QProcess;
class FileCompresser : public QObject
{
    Q_OBJECT
public:
    explicit FileCompresser(QObject *parent = nullptr);

    bool compress(QStringList const& fileNames,
                  CompressParam const& param,
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
    void setArgs(QStringList & args, CompressParam const& param);
    QStringList getFileNames(QString const&fileName,
                          CompressParam const& param);
    QString getFileName(QString const& fileName, QString const& newSuffix);
    QString getNewFileName(QString const& targetFilePath, QString const& fileName);
private:
    QProcess* process;
    int currentIndex;
    QList<QStringList> argsList;
    QStringList targetFileNames;
};

#endif // FILECOMPRESSER_H
