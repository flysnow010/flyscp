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
    bool isFinished() const;
signals:
    void progress(QString const& text);
    void error(QString const& error);
    void finished();
public slots:
    void onDataReady();
private:
    QString errorToText(int errorCode);
private:
    QProcess* process;
    int compressCount;
};

#endif // FILECOMPRESSER_H
