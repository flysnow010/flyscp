#include "clipboard.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QStringList>

#include <oleidl.h>
#define FILE_URL_HEADER "file:///"
ClipBoard::ClipBoard()
{

}

bool ClipBoard::canPaste()
{
    const QMimeData *mineData = QApplication::clipboard()->mimeData();
    if(mineData && mineData->hasFormat("Preferred DropEffect")
        && mineData->text().startsWith(FILE_URL_HEADER))
        return true;
    return false;
}

void ClipBoard::clear()
{
    QApplication::clipboard()->clear();
}

void ClipBoard::copy(QString const& text)
{
    QApplication::clipboard()->setText(text);
}

uint32_t ClipBoard::dropEffect()
{
    const QMimeData *mineData = QApplication::clipboard()->mimeData();
    QByteArray data = mineData->data("Preferred DropEffect");
    if(data.size() != 4)
        return 0;
    return *((uint32_t *)data.data());
}

bool ClipBoard::isCut(uint32_t dropEffect)
{
    if(dropEffect & DROPEFFECT_MOVE)
        return true;
    return false;
}

bool ClipBoard::isCopy(uint32_t dropEffect)
{
    if(dropEffect & DROPEFFECT_COPY)
        return true;
    return false;
}

QStringList ClipBoard::fileNames(QStringList const& fileNames)
{
    QStringList newFileNames;
    foreach(auto const& fileName, fileNames)
        newFileNames << FILE_URL_HEADER + fileName;
    return newFileNames;
}

QStringList ClipBoard::fileNames(const QMimeData *mimeData)
{
    QString text = mimeData->text().remove(FILE_URL_HEADER);
    if(text.endsWith("\n"))
        text.remove(text.size() - 1, 1);
    return text.split("\n");
}

QStringList ClipBoard::fileNames()
{
    return fileNames(QApplication::clipboard()->mimeData());
}

QString ClipBoard::remoteSrc(const QMimeData *mimeData)
{
    return QString::fromUtf8(mimeData->data("RemoteSrc"));
}

QString ClipBoard::compressedFileName(const QMimeData *mimeData)
{
    return QString::fromUtf8(mimeData->data("compressedFileName"));
}

