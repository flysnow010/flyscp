#ifndef DIRHISTORY_H
#define DIRHISTORY_H
#include <QStringList>
#include <QDir>

class DirHistory
{
public:
    DirHistory();
    enum { MAX_SIZE = 25 };

    inline void add(QString const& dir)
    {
        int index = dirs_.indexOf(dir);
        if(index >= 0)
            dirs_ << dirs_.takeAt(index);
        else
        {
            if(dirs_.size() >= MAX_SIZE)
                dirs_.takeFirst();
            dirs_ << dir;
        }
    }

    inline QString find(QString const& prefix) const
    {
        for(auto dir = dirs_.rbegin(); dir != dirs_.rend(); ++dir)
        {
            if(dir->startsWith(prefix))
                return *dir;
        }
        return QString("%1:/").arg(prefix);
    }

    inline QString next(QString const& dir) const
    {
        int index = dirs_.indexOf(dir);
        if(index < 0 || index  + 1 >= dirs_.size())
            return QString();

        for(int i = index + 1; i < dirs_.size(); i++)
        {
            QString nextDir = dirs_.at(i);
            if(QDir(nextDir).exists())
                return nextDir;
        }
        return QString();
    }

    inline QString pre(QString const& dir) const
    {
        int index = dirs_.indexOf(dir);
        if(index <= 0)
            return QString();

        for(int i = index - 1; i >= 0; i--)
        {
            QString preDir = dirs_.at(i);
            if(QDir(preDir).exists())
                return preDir;
        }
        return QString();
    }

    inline QStringList const& dirs() const { return dirs_; }

    inline void setDirs(QStringList const& dirs) { dirs_ = dirs; }
private:
    QStringList dirs_;
};

#endif // DIRHISTORY_H
