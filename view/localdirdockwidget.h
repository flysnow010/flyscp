#ifndef LOCALDIRDOCKWIDGET_H
#define LOCALDIRDOCKWIDGET_H
#include "core/filename.h"
#include "core/basedir.h"
#include <QDockWidget>

namespace Ui {
class LocalDirDockWidget;
}
class LocalDirModel;
class QMimeData;
class LocalDirDockWidget : public QDockWidget, public BaseDir
{
    Q_OBJECT

public:
    explicit LocalDirDockWidget(QWidget *parent = nullptr);
    ~LocalDirDockWidget();

    bool isRemote() const override { return false; }
    void setDir(QString const& dir) override;
    QString dir() const override;
    void cd(QString const& dir) override;

    void resizeSection(int logicalIndex, int size);
    void saveSettings(QString const& name);
    void loadSettings(QString const& name);
signals:
    void sectionResized(int logicalIndex, int oldSize, int newSize);

protected:
     bool eventFilter(QObject *obj, QEvent *event) override;
private slots:
    void viewClick(QModelIndex const& index);
    void sortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void customContextMenuRequested(const QPoint &pos);
    void beginDragFile(QPoint const& point);
    void dragEnter(QDragEnterEvent * event);
    void dragMove(QDragMoveEvent * event);
    void drop(QDropEvent * event);

    void cut();
    void copy();
    void paste();
    void delfile();
    void rename();
    void properties();
private:
    QStringList selectedileNames(QString const& prefix = QString());
    QString selectFileName();
    FileNames getFileNames(QStringList const& fileNames, QString const& filePath);
    void copyFilels(QStringList const& fileNames);
    void cutFiles(QStringList const& fileNames);
    void fileTransfer(FileNames const& fileNames, bool isMove);
private:
    Ui::LocalDirDockWidget *ui;
    LocalDirModel* model_;
};

#endif // LOCALDIRDOCKWIDGET_H
