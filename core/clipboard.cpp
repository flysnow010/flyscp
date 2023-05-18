#include "clipboard.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QStringList>

#include <oleidl.h>

ClipBoard::ClipBoard()
{

}

bool ClipBoard::canPaste()
{
    const QMimeData *mineData = QApplication::clipboard()->mimeData();
    if(mineData && mineData->hasFormat("Preferred DropEffect")
        && mineData->text().startsWith("file:///"))
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

QStringList ClipBoard::fileNames()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    return mimeData->text().split("\n");
}

