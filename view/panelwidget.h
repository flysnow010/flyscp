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
    void refresh();
    void retranslateUi();
signals:
    void tabCountChanged(int count);
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
private:
    void initDrivers();
    void updateDir(QString const& driver);
private:
    Ui::PanelWidget *ui;
    QButtonGroup* buttonGroup;
    QLabel* labelDiskInfo;
    bool isShowTips_;
    bool isGotRoot_;
};

#endif // PANELWIDGET_H
