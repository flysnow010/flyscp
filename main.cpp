#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("FlyScp");
    QApplication::setApplicationVersion("V1.0.0");
    MainWindow::InstallTranstoirs();
    MainWindow w;
    w.show();
    return a.exec();
}

