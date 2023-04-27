#ifndef PANELWIDGET_H
#define PANELWIDGET_H

#include <QWidget>

namespace Ui {
class PanelWidget;
}

class QButtonGroup;
class QAbstractButton;
class PanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PanelWidget(QWidget *parent = nullptr);
    ~PanelWidget();

    void addLocalDir(QWidget* widget, QString const& text = "Local");
    void addremoteDir(QWidget* widget, QString const& text = "Remote");
    void updateTexts(QWidget* widget);

private slots:
    void dirverChanged(QAbstractButton* button, bool checked);
    void backToRoot();
    void backToPrePath();
private:
    void updateDrivers();
    void updateDir(QString const& driver);
private:
    Ui::PanelWidget *ui;
    QButtonGroup* buttonGroup;
    bool isChecked  = false;
};

#endif // PANELWIDGET_H
