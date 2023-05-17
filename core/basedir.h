#ifndef BASEDIR_H
#define BASEDIR_H
#include <QString>
class BaseDir
{
public:
    BaseDir();

    virtual bool isRemote() const = 0;
    virtual void setDir(QString const& dir, QString const& caption = QString(), bool isNavigation = false) = 0;
    virtual QString dir() const = 0;
    virtual void cd(QString const& dir) = 0;
    virtual QString home() const = 0;
    virtual QString root() const = 0;
};

#endif // BASEDIR_H
