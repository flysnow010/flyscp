#ifndef COMMANDBAR_H
#define COMMANDBAR_H

#include <QFrame>

namespace Ui {
class CommandBar;
}

class CommandBar : public QFrame
{
    Q_OBJECT

public:
    explicit CommandBar(QWidget *parent = nullptr);
    ~CommandBar();

    void setDir(QString const& dir);
    void setDirFont(QFont const& font);
    void setActived();
signals:
    void commanded(QString const& commnad);
protected:
     bool eventFilter(QObject *obj, QEvent *event) override;
private:
    Ui::CommandBar *ui;
};

#endif // COMMANDBAR_H
