#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class PanelWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void saveSettings();
    void loadSettings();

    void createConnects();
private:
    Ui::MainWindow *ui;
    PanelWidget* leftPanelWidget;
    PanelWidget* rightPanelWidget;
};
#endif // MAINWINDOW_H
