#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class PanelWidget;
class LocalDirDockWidget;
class ToolButtons;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void save();
    void load();
    void saveSettings();
    void loadSettings();
    void loadStyleSheet();
    void createConnects();
private slots:
    void newFolder();
    void viewFile();
    void editFile();
    void copyFiles();
    void moveFiles();
    void delFiles();
    void selectAll();
private:
    Ui::MainWindow *ui;
    PanelWidget* leftPanelWidget;
    PanelWidget* rightPanelWidget;
    LocalDirDockWidget* leftDirView;
    LocalDirDockWidget* rightDirView;
    ToolButtons* toolButtons;
};
#endif // MAINWINDOW_H
