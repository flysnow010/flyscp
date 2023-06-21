#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class PanelWidget;
class LocalDirDockWidget;
class ToolButtons;
class SSHSettings;
class SSHSettingsManager;
class StatusBar;
class CommandBar;
class LayoutOption;
class DisplayOption;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
private slots:
    void newFolder();
    void newFile();
    void viewFile();
    void editFile();
    void copyFiles();
    void moveFiles();
    void delFiles();
    void selectAll();
    void compressFiles();
    void uncompressFiles();
    void searchFiles();
    void diffFolders();
    void diffFiles();
    void connectSftp();
    void options();
private:
    void save();
    void load();
    void saveSettings();
    bool loadSettings();
    void loadStyleSheet();
    void createConnects();
    void createMenuConnect();
    void createViewConnect();
    void createButtonsConnect();
    void createHelpMenu();
    void createDiffMenu();
    void createToolButtons();
    void createRemoteDirWidget(SSHSettings const& settings);
    void updateConnectMenu();
    void updateLayout(LayoutOption const& option);
    void updateDisplay(DisplayOption const& option);
private:
    Ui::MainWindow *ui;
    SSHSettingsManager *sshSettingsMangaer_;
    QMenu* connectMenu;
    QMenu* diffMenu;
    PanelWidget* leftPanelWidget;
    PanelWidget* rightPanelWidget;
    LocalDirDockWidget* leftDirView;
    LocalDirDockWidget* rightDirView;
    StatusBar* statusBar;
    CommandBar* commandBar;
    ToolButtons* toolButtons;
    bool isShowTips_;
};
#endif // MAINWINDOW_H
