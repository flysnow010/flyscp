#ifndef BASEDIR_H
#define BASEDIR_H
#include <QString>
#include <QFont>

class BaseDir
{
public:
    BaseDir();

    virtual bool isRemote() const = 0;
    virtual void setDir(QString const& dir,
                        QString const& caption = QString(),
                        bool isNavigation = false) = 0;
    virtual QString dir() const = 0;
    virtual QString findDir(QString const& prefix) const = 0;
    virtual void cd(QString const& dir) = 0;
    virtual void preDir() = 0;
    virtual void nextDir() = 0;
    virtual QString home() const = 0;
    virtual QString root() const = 0;
    virtual void refresh() = 0;
    virtual void showHeader(bool isShow) = 0;
    virtual void showCurrentDir(bool isShow) = 0;
    virtual void showDeskNavigationButton(bool isShow) = 0;
    virtual void showFavoriteButton(bool isShow) = 0;
    virtual void showHistoryButton(bool isShow) = 0;
    virtual void showHiddenAndSystem(bool isShow) = 0;
    virtual void showToolTips(bool isShow) = 0;
    virtual void showParentInRoot(bool isShow) = 0;
    virtual void setDirSoryByTime(bool isOn) = 0;
    virtual void setRenameFileName(bool isOn) = 0;
    virtual void showAllIconWithExeAndLink(bool isShow) = 0;
    virtual void showAllIcon(bool isShow) = 0;
    virtual void showStandardIcon(bool isShow) = 0;
    virtual void showNoneIcon(bool isShow) = 0;
    virtual void showIconForFyleSystem(bool isShow) = 0;
    virtual void showIconForVirtualFolder(bool isShow) = 0;
    virtual void showOverlayIcon(bool isShow) = 0;
    virtual void fileIconSize(int size) = 0;
    virtual void fileFont(QFont const& font) = 0;
    virtual void setItemColor(QString const& fore,
                              QString const& back,
                              QString const& alternate) = 0;
    virtual void setItemSelectedColor(QString const& back,
                                      QString const& mark,
                                      QString const&cursor) = 0;
    virtual void execCommand(QString const& command) = 0;
    virtual void viewFile() = 0;
    virtual void newFolder() = 0;
    virtual void newTxtFile() = 0;
    virtual void deleteFiles() = 0;
    virtual void selectAll() = 0;
    virtual void searchFiles(QString const& dir) = 0;
    virtual void setActived(bool isActived) = 0;
    virtual bool isActived() const = 0;
    virtual void retranslateUi() = 0;
};

#endif // BASEDIR_H
