#ifndef LOCALDIRDOCKWIDGET_H
#define LOCALDIRDOCKWIDGET_H
#include "core/filename.h"
#include <QDockWidget>

namespace Ui {
class LocalDirDockWidget;
}
class LocalDirModel;
class QMimeData;
class LocalDirDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit LocalDirDockWidget(QWidget *parent = nullptr);
    ~LocalDirDockWidget();

    void setDir(QString const& dir);
    QString dir();

    void cd(QString const& dir);
protected:
     bool eventFilter(QObject *obj, QEvent *event) override;
private slots:
    void viewClick(QModelIndex const& index);
    void sortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void customContextMenuRequested(const QPoint &pos);

    void cut();
    void copy();
    void paste();
    void delfile();
    void rename();
    void property();
private:
    QStringList selectedileNames(QString const& prefix = QString());
    QString selectFileName();
    void copyFilels(QStringList const& fileNames);
    void cutFiles(QStringList const& fileNames);
    void fileTransfer(FileNames const& fileNames, bool isMove);
private:
    Ui::LocalDirDockWidget *ui;
    LocalDirModel* model_;
};

#endif // LOCALDIRDOCKWIDGET_H
