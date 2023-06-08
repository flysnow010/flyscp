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
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
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
    void diffFiles();
    void connectSftp();
private:
    void save();
    void load();
    void saveSettings();
    void loadSettings();
    void loadStyleSheet();
    void createConnects();
    void createMenuConnect();
    void createViewConnect();
    void createButtonsConnect();
    void createHelpMenu();
    void createConnectButton();
    void createRemoteDirWidget(SSHSettings const& settings);
    void updateConnectMenu();
private:
    Ui::MainWindow *ui;
    SSHSettingsManager *sshSettingsMangaer_;
    QMenu* connectMenu;
    PanelWidget* leftPanelWidget;
    PanelWidget* rightPanelWidget;
    LocalDirDockWidget* leftDirView;
    LocalDirDockWidget* rightDirView;
    ToolButtons* toolButtons;
};
#endif // MAINWINDOW_H
