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
#include <QDebug>
void ClipBoard::test()
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    //qDebug() << mimeData->formats();

    qDebug() << "Shell IDList Array: " << mimeData->data("Shell IDList Array");
    qDebug() << "DataObjectAttributes: " << mimeData->data("DataObjectAttributes");
    qDebug() << "DataObjectAttributesRequiringElevation: "
        << mimeData->data("DataObjectAttributesRequiringElevation");
    qDebug() << "UIDisplayed: " << mimeData->data("UIDisplayed");
    qDebug() << "DropDescription: " << mimeData->data("DropDescription");
    qDebug() << "FileName: " << mimeData->data("FileName");
    qDebug() << "FileNameW: " << mimeData->data("FileNameW");
    qDebug() << "Shell Object Offsets: " << mimeData->data("Shell Object Offsets");
    qDebug() << "Preferred DropEffect: " << mimeData->data("Preferred DropEffect");
    qDebug() << "AsyncFlag: " << mimeData->data("AsyncFlag");

}

void ClipBoard::cut(QStringList const& fileNames)
{
    QString text = fileNames.join("\n");
    if(fileNames.size() > 1)
        text += "\n";

    QMimeData* mineData = new QMimeData();
    mineData->setText(text);
    QByteArray data(4, 0);
    data[0] = 2;
    mineData->setData("Preferred DropEffect", data);
    QApplication::clipboard()->setMimeData(mineData);
}

void ClipBoard::copy(QStringList const& fileNames)
{
    QString text = fileNames.join("\n");
    if(fileNames.size() > 1)
        text += "\n";

    QMimeData* mimeData = new QMimeData();
    mimeData->setText(text);
    QByteArray data(4, 0);
    data[0] = 5;
    mimeData->setData("Preferred DropEffect", data);
    QApplication::clipboard()->setMimeData(mimeData);
}

QMimeData* ClipBoard::copyMimeData(QStringList const& fileNames)
{
    QString text = fileNames.join("\n");
    if(fileNames.size() > 1)
        text += "\n";

    QMimeData* mineData = new QMimeData();
    mineData->setText(text);
    QByteArray data(4, 0);
    data[0] = 5;
    mineData->setData("Preferred DropEffect", data);
    return mineData;
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

