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

    void setButtonFont(QFont const& font);
    void retranslateUi();
signals:
    void switchClicked();
    void cmdClicked();
    void viewClicked();
    void editClicked();
    void copyClicked();
    void moveClicked();
    void newFolderClicked();
    void newFileClicked();
    void deleteClicked();
    void exitClicked();
private:
    Ui::ToolButtons *ui;
};

#endif // TOOLBUTTONS_H
