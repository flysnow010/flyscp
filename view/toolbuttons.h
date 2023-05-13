#ifndef TOOLBUTTONS_H
#define TOOLBUTTONS_H

#include <QFrame>

namespace Ui {
class ToolButtons;
}

class ToolButtons : public QFrame
{
    Q_OBJECT

public:
    explicit ToolButtons(QWidget *parent = nullptr);
    ~ToolButtons();

signals:
    void viewClicked();
    void editClicked();
    void copyClicked();
    void moveClicked();
    void newClicked();
    void deleteClicked();
    void exitClicked();
private:
    Ui::ToolButtons *ui;
};

#endif // TOOLBUTTONS_H
