#include "languagemanager.h"

LanguageManager::LanguageManager()
{
    languages_ << Language{ "English", "" }
               << Language{ "简体中文", "zh_CN" };
}

bool LanguageManager::find(QString const& name, Language & lang)
{
    foreach(auto language, languages_)
    {
        if(language.name == name)
        {
            lang = language;
            return true;
        }
    }
    return false;
}
