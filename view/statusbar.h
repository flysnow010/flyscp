#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QFrame>

namespace Ui {
class StatusBar;
}

class StatusBar : public QFrame
{
    Q_OBJECT

public:
    explicit StatusBar(QWidget *parent = nullptr);
    ~StatusBar();

    void setLeftWidth(int width);
    void setLeftStatusText(QString const& text);
    void setRightStatusText(QString const& text);
private:
    Ui::StatusBar *ui;
    int space;
};

#endif // STATUSBAR_H
