#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QIcon>
#include <QDir>

class Utils
{
public:
    static QString currentPath();
    static QString compressApp();
    static QString viewApp();
    static QString editApp();
    static QString diffApp();
    static QDir tempPath();
    static QByteArray readFile(QString const& filename);

    static QString formatTime(int time_ms);
    static QString formatDateTime(long long time_ms);
    static QString formatVideoTime(long long time_us);
    static QString formatVideoTime(int time_ms);

    static QIcon dirIcon();
    static QIcon computerIcon();
    static QIcon driverIcon();
    static QIcon networkIcon();
    static QIcon fileIcon(QString const& suffix);
    static QIcon GetIcon(QString const& fileName, int index = 0);
    static QString GetText(QString const& fileName, quint32 index);

    static QString formatFileSize(qint64 byte);
    static QString formatFileSizeB(qint64 byte);
    static QString formatFileSizeKB(qint64 byte);
    static QString formatFileSizeMB(qint64 byte);

    static QString permissionsText(quint32 permissions, bool isDir);
    static QString getText(QString const& label, QString const& value = QString());
    static QString getPassword(QString const& label);
    static bool question(QString const& text);
    static void warring(QString const& text);
};

#endif // UTILS_H
