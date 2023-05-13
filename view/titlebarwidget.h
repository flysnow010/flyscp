#ifndef TITLEBARWIDGET_H
#define TITLEBARWIDGET_H

#include <QWidget>

namespace Ui {
class TitleBarWidget;
}

class TitleBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBarWidget(QWidget *parent = nullptr);
    ~TitleBarWidget();

    void setTitle(QString const& title);
    void setActived(bool isActived);
    bool isActived() const { return actived_; }
signals:
    void libDirButtonClicked();
    void favoritesDirButtonCLicked();
    void historyDirButtonClicked();
private slots:
    void linkHovered(const QString &link);
private:
    Ui::TitleBarWidget *ui;
    bool actived_;
};

#endif // TITLEBARWIDGET_H
