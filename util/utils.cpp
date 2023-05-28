#include "utils.h"
#include "ssh/fileinfo.h"

#include <QApplication>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QDateTime>
#include <QDir>
#include <QtWin>
#include <QDebug>

#include <cstring>
#include <windows.h>

QString Utils::currentPath()
{
    return QApplication::applicationDirPath();
}

QString Utils::compressApp()
{
    return QString("%1/7z.exe").arg(QApplication::applicationDirPath());
}

QDir Utils::tempPath()
{
    QDir dir = QDir::temp();
    QString appName = QApplication::applicationName();
    dir.mkdir(appName);
    dir.cd(appName);
    return dir;
}

QByteArray Utils::readFile(QString const& filename)
{
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly))
        return file.readAll();
    return QByteArray();
}

QString Utils::formatTime(int time_ms)
{
    int const time_base = 1000;
    int hours, mins, secs, ms;
    secs = time_ms / time_base;
    ms = time_ms % time_base;
    mins = secs / 60;
    secs %= 60;
    hours = mins / 60;
    mins %= 60;

    QString strTime = "";
    if(hours > 0)
        strTime = QString("%1小时").arg(hours, 2, 10, QChar('0'));
    if(mins > 0)
        strTime = strTime + QString("%1分").arg(mins, 2, 10, QChar('0'));
    if(secs > 0)
        strTime = strTime + QString("%1秒").arg(secs, 2, 10, QChar('0'));

    if(strTime.isEmpty())
        strTime = QString("%1毫秒").arg(ms);
    return strTime;
}

QString Utils::formatDateTime(long long time_ms)
{
    return QDateTime::fromMSecsSinceEpoch(time_ms).toString("yyyy-MM-dd HH:mm:ss");
}

QString Utils::formatVideoTime(long long time_us)
{
    int const time_base = 1000000;
    int hours, mins, secs, us;
    secs = time_us / time_base;
    us = time_us % time_base;
    mins = secs / 60;
    secs %= 60;
    hours = mins / 60;
    mins %= 60;
    char text[40];
    sprintf_s(text, sizeof(text), "%02d:%02d:%02d:%02d", hours, mins, secs, us / 40000);
    return  QString(text);
}

QString Utils::formatVideoTime(int time_ms)
{
    int const time_base = 1000;
    int hours, mins, secs, ms;
    secs = time_ms / time_base;
    ms = time_ms % time_base;
    mins = secs / 60;
    secs %= 60;
    hours = mins / 60;
    mins %= 60;
    char text[40];
    sprintf_s(text, sizeof(text), "%02d:%02d:%02d:%02d", hours, mins, secs, ms / 40);
    return  QString(text);
}

QIcon Utils::dirIcon()
{
    QFileIconProvider fip;
    return fip.icon(QFileIconProvider::Folder);
}

QIcon Utils::computerIcon()
{
    QFileIconProvider fip;
    return fip.icon(QFileIconProvider::Computer);
}

QIcon Utils::driverIcon()
{
    QFileIconProvider fip;
    return fip.icon(QFileIconProvider::Drive);
}

QIcon Utils::networkIcon()
{
    QFileIconProvider fip;
    return fip.icon(QFileIconProvider::Network);
}

QIcon Utils::fileIcon(QString const& suffix)
{
    QFileIconProvider fip;
    if(suffix.isEmpty())
        return fip.icon(QFileIconProvider::File);
    else
    {
        QString strTemplateName = QDir::tempPath() + QDir::separator() +
                               QCoreApplication::applicationName() + "_XXXXXX." + suffix;
        QTemporaryFile tmpFile(strTemplateName);
        tmpFile.setAutoRemove(false);
        if (tmpFile.open())
        {
            tmpFile.close();
            return fip.icon(QFileInfo(tmpFile.fileName()));
        }
    }
    return QIcon();
}

QIcon Utils::GetIcon(QString const& fileName, int index)
{
    HICON hicon;
    if(ExtractIconEx(fileName.toStdWString().c_str(), index, &hicon, 0, 1) > 0)
    {
        if(hicon)
            return QIcon(QtWin::fromHICON(hicon));
        qDebug() << fileName <<  "," << index;
    }
    return QIcon();
}

QString Utils::GetText(QString const& fileName, quint32 index)
{
    HMODULE h = LoadLibrary(fileName.toStdWString().c_str());
    //GetModuleHandleEx(0x02, fileName.toStdWString().c_str(), &h);
    wchar_t text[256];
    if(!h)
    {
         qDebug() << fileName <<  "," << index;
         return QString();
    }
    int len = LoadString(h, index, text, 256);
    FreeLibrary(h);
    return QString::fromStdWString(std::wstring(text, len));
}

qint64 const SIZE_KB = 0x400;
qint64 const SIZE_MB = 0x100000;
qint64 const SIZE_GB = 0x40000000;
qint64 const SIZE_TB = 0x10000000000;

QString Utils::formatFileSize(qint64 byte)
{
    if(byte < SIZE_KB)
        return QString("%1 Bytes").arg(byte);
    else if(byte >= SIZE_KB  && byte < SIZE_MB)
        return QString("%1 KB").arg(static_cast<double>(byte) / SIZE_KB, 0, 'f', 2);
    else if(byte >= SIZE_MB && byte < SIZE_GB)
        return QString("%1 MB").arg(static_cast<double>(byte) / SIZE_MB, 0, 'f', 2);
    else if(byte >= SIZE_GB && byte < SIZE_TB)
        return QString("%1 GB").arg(static_cast<double>(byte) / SIZE_GB, 0, 'f', 2);
    else
        return QString("%1 TB").arg(static_cast<double>(byte) / SIZE_TB, 0, 'f', 2);
}

QString Utils::formatFileSizeB(qint64 byte)
{
    QStringList sizes;
    qint64 size = byte;
    while(size > 1000)
    {
        sizes.push_front(QString("%1").arg(size % 1000));
        size /= 1000;
    }
    if(size)
        sizes.push_front(QString("%1").arg(size));
    if(byte == 0)
        return QString("0");
    return sizes.join(",");
}

QString Utils::formatFileSizeKB(qint64 byte)
{
    return QString("%1 KB").arg(formatFileSizeB((byte +SIZE_KB - 1) / SIZE_KB));
}

QString Utils::formatFileSizeMB(qint64 byte)
{
    return QString("%1 MB").arg(formatFileSizeB((byte +SIZE_MB - 1) / SIZE_MB));
}

QString Utils::permissionsText(quint32 permissions, bool isDir)
{
    QString p;
    p += isDir   ? "d" : "-";
    p += (permissions & ssh::FileInfo::User_Read)   ? "r" : "-";
    p += (permissions & ssh::FileInfo::User_Write)  ? "w" : "-";
    p += (permissions & ssh::FileInfo::User_Exe)    ? "x" : "-";
    p += (permissions & ssh::FileInfo::Group_Read)  ? "r" : "-";
    p += (permissions & ssh::FileInfo::Group_Write) ? "w" : "-";
    p += (permissions & ssh::FileInfo::Group_Exe)   ? "x" : "-";
    p += (permissions & ssh::FileInfo::Other_Read)  ? "r" : "-";
    p += (permissions & ssh::FileInfo::Other_Write) ? "w" : "-";
    p += (permissions & ssh::FileInfo::Other_Exe)   ? "x" : "-";

    return p;
}

QString Utils::getText(QString const& label, QString const& value)
{
    QInputDialog dialog;
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setWindowTitle(QApplication::applicationName());
    dialog.setLabelText(label);
    dialog.setTextValue(value);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    if(dialog.exec() == QDialog::Accepted)
        return dialog.textValue();
    return QString();
}

bool Utils::question(QString const& text)
{
    if(QMessageBox::question(0, QApplication::applicationName(), text) == QMessageBox::Yes)
        return true;
    return false;
}

void Utils::warring(QString const& text)
{
    QMessageBox::warning(0, QApplication::applicationName(), text);
}
