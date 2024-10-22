#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class PanelWidget;
class LocalDirDockWidget;
class ToolButtons;
class SSHSettings;
class SSHSettingsManager;
class UserAuthManager;
class StatusBar;
class CommandBar;
class LayoutOption;
class DisplayOption;
class IconsOption;
class FontOption;
class ColorOption;
class LanguageOption;
class OperationOption;
class QTranslator;
class QLockFile;
class QAction;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static void InstallTranstoirs(bool isInited = false);
    static bool IsRunSingle();
protected:
    bool nativeEvent(const QByteArray &eventType,
                     void *message,
                     long *result) override;
    bool eventFilter(QObject *obj,
                     QEvent *event) override;
private slots:
    void newFolder();
    void newFile();
    void switchView();
    void cmdView();
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
    void netsettings();
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
    void connectToSSh(SSHSettings & settings);
    void createRemoteDirWidget(SSHSettings const& settings, bool isSavePassword);
    void updateConnectMenu();
    void updateLayout(LayoutOption const& option);
    void updateDisplay(DisplayOption const& option, bool isRefresh = true);
    void updateIcons(IconsOption const& option, bool isRefresh = true);
    void updateFonts(FontOption const& option);
    void updateColors(ColorOption const& option, bool isRefresh = true);
    void updateOperation(OperationOption const& option);
private:
    Ui::MainWindow *ui;
    static QTranslator appTranslator;
    static QTranslator sysTranslator;
    SSHSettingsManager *sshSettingsMangaer_;
    UserAuthManager* userAuthManager_;
    QMenu* connectMenu;
    QMenu* diffMenu;
    QAction* diffFilesAction = 0;
    QAction* netsettingsAction = 0;
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
