#ifndef PANELWIDGET_H
#define PANELWIDGET_H

#include <QWidget>

namespace Ui {
class PanelWidget;
}

class QButtonGroup;
class QAbstractButton;
class QIcon;
class QLabel;
class DiskManagerController;
class PanelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PanelWidget(QWidget *parent = nullptr);
    ~PanelWidget();

    void addDirTab(QWidget* widget,
                   QIcon const& icon,
                   QString const& text);
    void updateTexts(QWidget* widget);

    bool isRemote() const;
    QString currentDir() const;
    QStringList selectedFileNames() const;
    void preDir();
    void nextDir();

    void updateDrivers(bool isAdded);
    int tabCount() const;

    void showDriveButtons(bool isShow);
    void showHeader(bool isShow);
    void showCurrentDir(bool isShow);
    void showDeskNavigationButton(bool isShow);
    void showFavoriteButton(bool isShow);
    void showHistoryButton(bool isShow);
    void showHiddenAndSystem(bool isShow);
    void showToolTips(bool isShow);
    void showDriveToolTips(bool isShow);
    void showParentInRoot(bool isShow);
    void setDirSortByTime(bool isOn);
    void setRenameFileName(bool isOn);
    void showAllIconWithExeAndLink(bool isShow);
    void showAllIcon(bool isShow);
    void showStandardIcon(bool isShow);
    void showNoneIcon(bool isShow);
    void showIconForFyleSystem(bool isShow);
    void showIconForVirtualFolder(bool isShow);
    void showOverlayIcon(bool isShow);
    void fileIconSize(int size);
    void fileFont(QFont const& font);
    void setDriveFont(QFont const& font);
    void setItemColor(QString const& fore,
                      QString const& back,
                      QString const&alternate);
    void setItemSelectedColor(QString const& back,
                      QString const& mark,
                      QString const&cursor);
    void setGoToRoot(bool isOn) { isGotRoot_ = isOn; }

    void execCommand(QString const& command);
    void viewFile();
    void newFolder();
    void newTxtFile();
    void deleteFiles(bool isPrompt = true);
    void selectAll();
    void uploadFiles(QStringList const& fileNames);
    void copyFiles(QString const& dstFilePath);
    void moveFiles(QString const& dstFilePath);
    void searchFiles(QString const& dstFilePath);
    bool isActived() const;
    bool setActived();
    void setUnActived();
    void refreshCurrent();

    void refresh();
    void retranslateUi(QString const& tabText);
signals:
    void tabCountChanged(int count);
    void tabActived(QString const& dir);
public slots:
    void setTabBarAutoHide(int count);
    void upddateDrive();
    void libDirContextMenu();
    void closeTab(QWidget *w);
protected:
    bool eventFilter(QObject *obj,
                     QEvent *event) override;
private slots:
    void buttonClicked(QAbstractButton* button);
    void backToHome();
    void backToRoot();
    void backToPrePath();
    void currentChanged(int index);
    void tabCloseRequested(int index);
    void ejectDiskFinished(QString const& result);
private:
    void driverContextMenu(QString const& driver);
    void initDrivers();
    void updateDir(QString const& driver);
private:
    Ui::PanelWidget *ui;
    QButtonGroup* buttonGroup;
    QLabel* labelDiskInfo;
    DiskManagerController *diskManagerController;
    bool isShowTips_;
    bool isGotRoot_;
};

#endif // PANELWIDGET_H
