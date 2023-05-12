#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class PanelWidget;
class LocalDirDockWidget;
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
private:
    Ui::MainWindow *ui;
    PanelWidget* leftPanelWidget;
    PanelWidget* rightPanelWidget;
    LocalDirDockWidget* leftDirView;
    LocalDirDockWidget* rightDirView;
};
#endif // MAINWINDOW_H
