#ifndef CLIPBOARD_H
#define CLIPBOARD_H
#include <cstdint>

class QStringList;
class QString;
class QMimeData;
class ClipBoard
{
public:
    ClipBoard();

    static bool canPaste();
    static void clear();
    static void copy(QString const& text);
    static uint32_t dropEffect();
    static bool isCut(uint32_t dropEffect);
    static bool isCopy(uint32_t dropEffect);
    static QStringList fileNames(QStringList const& fileNames);
    static QStringList fileNames(const QMimeData *mimeData);
    static QStringList fileNames();
};

#endif // CLIPBOARD_H
