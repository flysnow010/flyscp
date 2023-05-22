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
class TitleBarWidget;
class LocalDirDockWidget : public QDockWidget, public BaseDir
{
    Q_OBJECT

public:
    explicit LocalDirDockWidget(QWidget *parent = nullptr);
    ~LocalDirDockWidget();

    bool isRemote() const override { return false; }
    void setDir(QString const& dir, QString const& caption = QString(), bool isNavigation  = false) override;
    QString dir() const override;
    void cd(QString const& dir) override;
    QString home() const override;
    QString root() const override;

    void resizeSection(int logicalIndex, int size);
    void saveSettings(QString const& name);
    void loadSettings(QString const& name);
    void setActived(bool isActived);
    bool isActived() const;

    void viewFile();
    void editFile();
    void copyFiles(QString const& dstFilePath);
    void moveFiles(QString const& dstFilePath);
    void refresh();
    void delFilesWithConfirm();
    void selectAll();
public slots:
    void newFolder();
    void newTxtFile();

signals:
    void sectionResized(int logicalIndex, int oldSize, int newSize);
    void dirChanged(QString const& dir, bool isNavigation);
    void libDirContextMenuRequested();
    void favoritesDirContextMenuRequested();
    void historyDirContextMenuRequested();
    void actived();

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
    void delFiles();
    void rename();

    void createShortcut();
private:
    bool isMultiSelected();
    QStringList selectedileNames(QString const& prefix = QString(),
                                 bool isOnlyFilename = false, bool isParent = false);
    QString selectedFileName(bool isOnlyFilename = false);

    void copyFilels(QStringList const& fileNames, QString const& dstFilePath);
    void cutFiles(QStringList const& fileNames, QString const& dstFilePath);
    void fileTransfer(FileNames const& fileNames, bool isMove);
    void updateCurrentDir(QString const& dir, QString const& caption = QString(), bool isNavigation = false);
private:
    Ui::LocalDirDockWidget *ui;
    LocalDirModel* model_;
    TitleBarWidget* titleBarWidget;
};

#endif // LOCALDIRDOCKWIDGET_H
