#ifndef LOCALDIRDOCKWIDGET_H
#define LOCALDIRDOCKWIDGET_H
#include "core/filename.h"
#include "core/basedir.h"
#include "core/compressfileinfo.h"
#include <QDockWidget>

namespace Ui {
class LocalDirDockWidget;
}
class LocalDirModel;
class CompressDirModel;
class QMimeData;
class TitleBarWidget;
class QItemSelectionModel;
class DirModel;
class DirFavorite;
class DirHistory;
class QFileSystemWatcher;
class LocalDirDockWidget : public QDockWidget, public BaseDir
{
    Q_OBJECT

public:
    enum ShowMode { Normal, Compress };
    explicit LocalDirDockWidget(QWidget *parent = nullptr);
    ~LocalDirDockWidget();

    bool isRemote() const override { return false; }
    void setDir(QString const& dir,
                QString const& caption = QString(),
                bool isNavigation  = false) override;
    QString dir() const override;
    QStringList selectedFileNames() const override;
    QString findDir(QString const& prefix) const override;
    void cd(QString const& dir) override;
    void preDir() override;
    void nextDir() override;
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
    void execCommand(QString const& command) override;
    void viewFile() override;
    void newFolder() override;
    void newTxtFile() override;
    void deleteFiles() override;
    void selectAll() override;
    void copyFiles(QString const& dstFilePath) override;
    void moveFiles(QString const& dstFilePath) override;
    void searchFiles(QString const& dstFilePath) override;
    void setActived(bool isActived) override;
    bool isActived() const override;
    void retranslateUi() override;

    QString currentFileName() const { return selectedFileName(); }
    void resizeSection(int logicalIndex, int size);
    void saveSettings(QString const& name);
    void loadSettings(QString const& name);

    void editFile();



    void compressFiles(QString const& dstFilePath);
    void uncompressFiles(QString const& dstFilePath);
    void extractFiles(QStringList const& fileNames,
                      QString const& targetPath);


signals:
    void sectionResized(int logicalIndex,
                        int oldSize,
                        int newSize);
    void dirChanged(QString const& dir);
    void statusTextChanged(QString const& text);
    void libDirContextMenuRequested();
    void favoritesDirContextMenuRequested();
    void historyDirContextMenuRequested();
    void copyRequested();
    void moveRequested();
    void remoteDownload(QString const& remoteSrc,
                        QStringList const& fileNames,
                        QString const& targetFilePath);
    void compressFileExtract(QStringList const& fileNames);
    void actived(QString const& dir);

protected:
     bool eventFilter(QObject *obj, QEvent *event) override;
private slots:
    void normalDoubleClick(QModelIndex const& index);
    void compressDoubleClick(QModelIndex const& index);
    void directoryChanged(const QString &path);
    void sortIndicatorChanged(int logicalIndex,
                              Qt::SortOrder order);
    void favoritesDirContextMenu();
    void historyDirContextMenu();
    void customNormalContextMenu(const QPoint &pos);
    void customCompressContextMenu(const QPoint &pos);

    void normalBeginDragFile(QPoint const& point);
    void compressBeginDragFile(QPoint const& point);
    void normalDragEnter(QDragEnterEvent * event);
    void compressDragEnter(QDragEnterEvent * event);
    void normalDragMove(QDragMoveEvent * event);
    void compressDragMove(QDragMoveEvent * event);
    void normalDrop(QDropEvent * event);
    void compressDrop(QDropEvent * event);

    void cut();
    void copy();
    void paste();
    void delFiles();
    void rename();

    void createShortcut();
private:
    bool isMultiSelected();
    bool isCompressFiles(QString const& suffix);
    QStringList getSelectedFileNames(bool isOnlyFilename = false,
                                  bool isParent = false) const;
    QStringList selectedCompressedFileNames();
    QString selectedFileName(bool isOnlyFilename = false) const;
    CompressFileInfo::Ptr selectedCompressedFileName();

    void copyFilels(QStringList const& fileNames,
                    QString const& dstFilePath);
    void cutFiles(QStringList const& fileNames,
                  QString const& dstFilePath);
    void fileTransfer(FileNames const& fileNames,
                      bool isMove);
    void extractFiles(QStringList const& fileNames,
                      QString const& targetPath,
                      bool isMove);
    void compressFiles(QStringList const& fileNames,
                       QString const& filePath);
    void goToFile(QString const& fileName);

    QString getStatusText();
    QString getStatusText(QItemSelectionModel* selectMode,
                          DirModel* model);
    void updateCurrentDir(QString const& dir,
                          QString const& caption = QString(),
                          bool isNavigation = false);
    ShowMode showMode() const { return showMode_ ; }
    void setShowMode(ShowMode const& mode);
private:
    Ui::LocalDirDockWidget *ui;
    LocalDirModel* model_;
    CompressDirModel* compressModel_;
    TitleBarWidget* titleBarWidget;
    QWidget*        hideBarWidget;
    DirFavorite* dirFavorite;
    DirHistory* dirHistory;
    QFileSystemWatcher* fileSystemWatcher;
    ShowMode showMode_ = Normal;
};

#endif // LOCALDIRDOCKWIDGET_H
