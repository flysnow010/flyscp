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
    explicit TitleBarWidget(bool isWindows = true, QWidget *parent = nullptr);
    ~TitleBarWidget();

    void setTitle(QString const& title);
    void setActived(bool isActived);
    bool isActived() const { return actived_; }

    void showLibDirButton(bool isShow);
    void showFavoriteButton(bool isShow);
    void showHistoryButton(bool isShow);
signals:
    void libDirButtonClicked();
    void favoritesDirButtonCLicked();
    void historyDirButtonClicked();
    void dirSelected(QString const& dir);
    void actived();
protected:
     bool eventFilter(QObject *obj, QEvent *event) override;
private slots:
    void linkHovered(const QString &link);
private:
    void selectDir(int x);
private:
    Ui::TitleBarWidget *ui;
    bool actived_;
    bool isWindows_;
};

#endif // TITLEBARWIDGET_H
