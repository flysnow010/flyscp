#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H
#include <QString>
#include <QList>

struct Language
{
    QString name;
    QString file;

    bool isDefault() const { return file.isEmpty(); }
    QString showText() const
    {
        if(file.isEmpty())
            return name;
        return QString("%1(%2.qm)").arg(name, file);
    }
};
typedef QList<Language> Languages;

class LanguageManager
{
public:
    LanguageManager();

    bool find(QString const& name, Language & lang);
    Languages const& languages() const { return languages_; }
private:
    Languages languages_;
};

#endif // LANGUAGEMANAGER_H
