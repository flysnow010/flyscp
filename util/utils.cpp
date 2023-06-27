#include "utils.h"
#include "ssh/fileinfo.h"

#include <QApplication>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QtWin>

#include <cstring>
#include <windows.h>

QString Utils::currentPath()
{
    QString filePath = QApplication::applicationDirPath();
    if(filePath.startsWith("C:") ||filePath.startsWith("c:"))
        return QString("%1/FlyScp").arg(QDir::homePath());
    return filePath;
}

QString Utils::tempPath()
{
    return QDir::tempPath();
}

QString Utils::sshSettingsPath()
{
    QString path = QString("%1/sshSettings").arg(currentPath());
    QDir dir(path);
    if(!dir.exists(path))
        dir.mkpath(path);
    return path;
}

QString Utils::compressApp()
{
    return QString("%1/7z.exe").arg(QApplication::applicationDirPath());
}

QString Utils::viewApp()
{
    return QString("%1/MobaRTE.exe").arg(QApplication::applicationDirPath());
}

QString Utils::editApp()
{
    return QString("%1/MobaRTE.exe").arg(QApplication::applicationDirPath());
}

QString Utils::diffApp()
{
    return QString("%1/MobaRTE.exe").arg(QApplication::applicationDirPath());
}

QDir Utils::tempDir()
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
    return QFileIconProvider().icon(QFileIconProvider::Folder);
}

QIcon Utils::fileIcon()
{
    return QFileIconProvider().icon(QFileIconProvider::File);
}

QIcon Utils::computerIcon()
{
    return QFileIconProvider().icon(QFileIconProvider::Computer);
}

QIcon Utils::driverIcon()
{
    return QFileIconProvider().icon(QFileIconProvider::Drive);
}

QIcon Utils::networkIcon()
{
    return QFileIconProvider().icon(QFileIconProvider::Network);
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
    }
    return QIcon();
}

QString Utils::GetText(QString const& fileName, quint32 index)
{
    HMODULE h = LoadLibrary(fileName.toStdWString().c_str());
    wchar_t text[256];
    if(!h)
         return QString();
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
        return QString("%1 GB").arg(static_cast<double>(byte) / SIZE_GB, 0, 'f', 1);
    else
        return QString("%1 TB").arg(static_cast<double>(byte) / SIZE_TB, 0, 'f', 1);
}

QString Utils::formatFileSizeB(qint64 byte)
{
    return QString("%L1").arg(byte);
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

QString Utils::getPassword(QString const& label)
{
    QInputDialog dialog;
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setWindowTitle(QApplication::applicationName());
    dialog.setLabelText(label);
    dialog.setTextEchoMode(QLineEdit::Password);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    if(dialog.exec() == QDialog::Accepted)
        return dialog.textValue();
    return QString();
}

QString Utils::getPath(QString const& caption)
{
    return QFileDialog::getExistingDirectory(0, caption);
}

QString Utils::getSaveFile(QString const& caption)
{
    return QFileDialog::getSaveFileName(0, caption);
}

bool Utils::question(QString const& text)
{
    if(QMessageBox::question(0, QApplication::applicationName(), text) == QMessageBox::Yes)
        return true;
    return false;
}

QString Utils::toWindowsPath(QString const& linuxPath)
{
   QString windowsPath = linuxPath;
   windowsPath.replace("/", "\\");
   return windowsPath;
}

QString Utils::toLinuxPath(QString const& windowsPath)
{
    QString linuxPath = windowsPath;
    linuxPath.replace("\\", "/");
    return linuxPath;
}

void Utils::warring(QString const& text)
{
    QMessageBox::warning(0, QApplication::applicationName(), text);
}

bool Utils::saveFile(QString const& fileName, QString const& text)
{
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly))
        return false;
    file.write(text.toUtf8());
    return true;
}
