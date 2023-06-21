#ifndef PANELWIDGET_H
#define PANELWIDGET_H

#include <QWidget>

namespace Ui {
class PanelWidget;
}

class QButtonGroup;
class QAbstractButton;
class QIcon;
class DirFavorite;
class DirHistory;
class QLabel;
class PanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PanelWidget(QWidget *parent = nullptr);
    ~PanelWidget();

    void addDirTab(QWidget* widget, QIcon const& icon, QString const& text);
    void updateTexts(QWidget* widget);

    void saveSettings(QString const& name);
    void loadSettings(QString const& name);
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
    void setDirSoryByTime(bool isOn);
signals:
    void tabCountChanged(int count);
public slots:
    void setTabBarAutoHide(int count);
    void addDirToHistory(QString const& dir, bool isNavigation);
    void libDirContextMenu();
    void favoritesDirContextMenu();
    void historyDirContextMenu();
    void closeTab(QWidget *w);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
private slots:
    void buttonClicked(QAbstractButton* button);
    void backToHome();
    void backToRoot();
    void backToPrePath();
    void currentChanged(int index);
    void tabCloseRequested(int index);
private:
    void initDrivers();
    void updateDir(QString const& driver);
private:
    Ui::PanelWidget *ui;
    QButtonGroup* buttonGroup;
    QLabel* labelDiskInfo;
    DirFavorite* dirFavorite;
    DirHistory* dirHistory;
    bool isShowTips_;
};

#endif // PANELWIDGET_H
