#ifndef COLORTOOLBUTTON_H
#define COLORTOOLBUTTON_H

#include <QToolButton>

class ColorToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ColorToolButton(QWidget *parent = nullptr);

    void setDefaultColor(QColor const& color);

signals:
    void selectedColor(QColor const& color);

private:
    QIcon createColorIcon(QColor const& color);
    QAction* createGridColorAction(QMenu* menu);
    QMenu* createColorMenu();
private:
    QMenu* colorMenu;
    QAction* defaultAction;
    QColor defaultColor;
};

#endif // COLORTOOLBUTTON_H
