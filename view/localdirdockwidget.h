#ifndef LOCALDIRDOCKWIDGET_H
#define LOCALDIRDOCKWIDGET_H
#include "core/filename.h"
#include "core/basedir.h"
#include <QDockWidget>

namespace Ui {
class LocalDirDockWidget;
}
class LocalDirModel;
class CompressDirModel;
class QMimeData;
class TitleBarWidget;
class QFileSystemWatcher;
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
    void refresh() override;
    void showHeader(bool isShow) override;
    void showCurrentDir(bool isShow) override;
    void showDeskNavigationButton(bool isShow) override;
    void showFavoriteButton(bool isShow)override;
    void showHistoryButton(bool isShow) override;
    void showHiddenAndSystem(bool isShow) override;
    void showToolTips(bool isShow) override;
    void showParentInRoot(bool isShow) override;
    void setDirSoryByTime(bool isOn) override;
    void setRenameFileName(bool isOn) override;
    void showAllIconWithExeAndLink(bool isShow) override;
    void showAllIcon(bool isShow) override;
    void showStandardIcon(bool isShow) override;
    void showNoneIcon(bool isShow) override;
    void showIconForFyleSystem(bool isShow) override;
    void showIconForVirtualFolder(bool isShow) override;
    void showOverlayIcon(bool isShow) override;
    void fileIconSize(int size) override;
    void fileFont(QFont const& font) override;
    void setItemColor(QString const& fore,
                      QString const& back,
                      QString const&alternate) override;
    void setItemSelectedColor(QString const& back,
                      QString const& mark,
                      QString const&cursor) override;

    QString currentFileName() const { return selectedFileName(); }
    void resizeSection(int logicalIndex, int size);
    void saveSettings(QString const& name);
    void loadSettings(QString const& name);
    void setActived(bool isActived);
    bool isActived() const;

    void viewFile();
    void editFile();
    void copyFiles(QString const& dstFilePath);
    void moveFiles(QString const& dstFilePath);

    void delFilesWithConfirm();
    void selectAll();
    void compressFiles(QString const& dstFilePath);
    void uncompressFiles(QString const& dstFilePath);
    void searchFiles(QString const& dstFilePath);
    void execCommand(QString const& command);
public slots:
    void newFolder();
    void newTxtFile();

signals:
    void sectionResized(int logicalIndex, int oldSize, int newSize);
    void dirChanged(QString const& dir, bool isNavigation);
    void statusTextChanged(QString const& text);
    void libDirContextMenuRequested();
    void favoritesDirContextMenuRequested();
    void historyDirContextMenuRequested();
    void remoteDownload(QString const& remoteSrc,
                        QStringList const& fileNames,
                        QString const& targetFilePath);
    void actived();

protected:
     bool eventFilter(QObject *obj, QEvent *event) override;
private slots:
    void normalDoubleClick(QModelIndex const& index);
    void compressDoubleClick(QModelIndex const& index);
    void directoryChanged(const QString &path);
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
    bool isCompressFiles(QString const& suffix);
    QStringList selectedFileNames(bool isOnlyFilename = false, bool isParent = false);
    QString selectedFileName(bool isOnlyFilename = false) const;

    void copyFilels(QStringList const& fileNames, QString const& dstFilePath);
    void cutFiles(QStringList const& fileNames, QString const& dstFilePath);
    void fileTransfer(FileNames const& fileNames, bool isMove);
    void goToFile(QString const& fileName);
    QString getStatusText();
    void updateCurrentDir(QString const& dir,
                          QString const& caption = QString(),
                          bool isNavigation = false);
private:
    Ui::LocalDirDockWidget *ui;
    LocalDirModel* model_;
    CompressDirModel* compressModel_;
    TitleBarWidget* titleBarWidget;
    QFileSystemWatcher* fileSystemWatcher;
};

#endif // LOCALDIRDOCKWIDGET_H
