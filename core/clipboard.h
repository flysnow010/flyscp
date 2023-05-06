#ifndef CLIPBOARD_H
#define CLIPBOARD_H
#include <cstdint>

class QStringList;
class QString;
class ClipBoard
{
public:
    ClipBoard();

    static bool canPaste();
    static void cut(QStringList const& fileNames);
    static void copy(QStringList const& fileNames);
    static void copy(QString const& text);
    static uint32_t dropEffect();
    static bool isCut(uint32_t dropEffect);
    static bool isCopy(uint32_t dropEffect);
    static QStringList fileNames();
};

#endif // CLIPBOARD_H
