#ifndef FILENAME_H
#define FILENAME_H
#include <QString>
#include <QList>

struct FileName
{
    QString src;
    QString dst;
};
typedef  QList<FileName> FileNames;
#endif
