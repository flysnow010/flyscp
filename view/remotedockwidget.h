#ifndef REMOTEDOCKWIDGET_H
#define REMOTEDOCKWIDGET_H
#include "core/sshsettings.h"
#include "core/filename.h"
#include "core/basedir.h"

#include <QDockWidget>

namespace Ui {
class RemoteDockWidget;
}
class RemoteDirModel;
class SFtpSession;
class TitleBarWidget;
class DirFavorite;
class DirHistory;
class QDir;
namespace ssh {
class FileInfoPtr;
}

class RemoteDockWidget : public QDockWidget, public BaseDir
{
    Q_OBJECT

public:
    explicit RemoteDockWidget(QWidget *parent = nullptr);
    ~RemoteDockWidget();

    enum OperateType{ Upload, Download, Delete };

    bool isRemote() const override { return true; }
    void setDir(QString const& dir,
                QString const& caption = QString(),
                bool  isNavigation = false) override;
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

    void start(SSHSettings const& settings);
    QString const& name() const { return name_; }

signals:
    void closeRequest(QWidget* widget);

public slots:
    void libDirContextMenu();
    void favoritesDirContextMenu();
    void historyDirContextMenu();
    void downloadFiles(QString const& remoteSrc,
                       QStringList const& fileNames,
                       QString const& targetFilePath);

private slots:
    void viewClick(QModelIndex const& index);
    void customContextMenuRequested(const QPoint &pos);
    void sortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void beginDragFile(QPoint const& point);
    void dragEnter(QDragEnterEvent * event);
    void dragMove(QDragMoveEvent * event);
    void drop(QDropEvent * event);
    void connected();
    void unconnected();
    void connectionError(QString const& error);

    void parentDirectory();
    void makeDirectory();
    void newFile();
    void upload();

    void open();
    void openWith();
    void download();
    void deleteFiles();
    void rename();
    void copyFilepath();
    void properties();
    void permissions();
private:
    void saveSettings();
    void loadSettings();

    QStringList selectedileNames();

    void openDir(ssh::FileInfoPtr const& fileInfo);
    QString download(ssh::FileInfoPtr const& fileInfo, QDir const& dstDir);
    bool upload(QString const& fileName);
    void fileTransfer(QStringList const& srcFileNames,
                      QString const& srcFilePath,
                      QString const& dstFilePath,
                      OperateType type);
    void updateCurrentDir(QString const& dir,
                          QString const& caption = QString(),
                          bool  isNavigation = false);
private:
    Ui::RemoteDockWidget *ui;
    RemoteDirModel* model_;
    TitleBarWidget* titleBarWidget;
    SFtpSession* sftp;
    DirFavorite* dirFavorite;
    DirHistory* dirHistory;
    QString name_;
    QString remoteID_;
};

#endif // REMOTEDOCKWIDGET_H
