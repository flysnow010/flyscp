#ifndef REMOTEDOCKWIDGET_H
#define REMOTEDOCKWIDGET_H
#include <core/sshsettings.h>
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

    bool isRemote() const override { return true; }
    void setDir(QString const& dir, QString const& caption = "") override;
    QString dir() const override;
    void cd(QString const& dir) override;
    QString home() const override;
    QString root() const override;

    void start(SSHSettings const& settings);
    QString const& name() const { return name_; }
public slots:
    void libDirContextMenu();
    void favoritesDirContextMenu();
    void historyDirContextMenu();
signals:
    void dirChanged(QString const& dir, bool isRemote);
private slots:
    void viewClick(QModelIndex const& index);
    void customContextMenuRequested(const QPoint &pos);
    void sortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void connected();
    void unconnected();
    void connectionError(QString const& error);

    void parentDirectory();
    void makeDirectory();
    void newFile();
    void refreshFolder();
    void upload();

    void open();
    void openWith();
    void download();
    void deleteDir();
    void deleteFile();
    void rename();
    void copyFilepath();
    void properties();
    void permissions();
private:
    void saveSettings();
    void loadSettings();
    QString getText(QString const& label, QString const& value = QString());
    void openDir(ssh::FileInfoPtr const& fileInfo);
    QString download(ssh::FileInfoPtr const& fileInfo, QDir const& dstDir);
    bool upload(QString const& fileName);
    void updateCurrentDir(QString const& dir, QString const& caption = "");
private:
    Ui::RemoteDockWidget *ui;
    RemoteDirModel* model_;
    TitleBarWidget* titleBarWidget;
    SFtpSession* sftp;
    DirFavorite* dirFavorite;
    DirHistory* dirHistory;
    QString name_;
};

#endif // REMOTEDOCKWIDGET_H
