#ifndef PANELWIDGET_H
#define PANELWIDGET_H

#include <QWidget>

namespace Ui {
class PanelWidget;
}

class QButtonGroup;
class QAbstractButton;
class QIcon;
class PanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PanelWidget(QWidget *parent = nullptr);
    ~PanelWidget();

    void addDirTab(QWidget* widget, QIcon const& icon, QString const& text);
    void updateTexts(QWidget* widget);

private slots:
    void dirverChanged(QAbstractButton* button, bool checked);
    void backToRoot();
    void backToPrePath();
    void currentChanged(int index);
private:
    void updateLocalDrivers();
    void remoteDirvers();
    void updateDir(QString const& driver);
private:
    Ui::PanelWidget *ui;
    QButtonGroup* buttonGroup;
    bool isChecked  = false;
};

#endif // PANELWIDGET_H
