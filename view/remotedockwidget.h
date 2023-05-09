#ifndef REMOTEDOCKWIDGET_H
#define REMOTEDOCKWIDGET_H
#include <core/sshsettings.h>
#include <QDockWidget>

namespace Ui {
class RemoteDockWidget;
}
class RemoteDirModel;
class SFtpSession;
class QDir;
namespace ssh {
class FileInfoPtr;
}

class RemoteDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit RemoteDockWidget(QWidget *parent = nullptr);
    ~RemoteDockWidget();

    void start(SSHSettings const& settings);
    QString const& name() const { return name_; }
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
private:
    Ui::RemoteDockWidget *ui;
    RemoteDirModel* model_;
    SFtpSession* sftp;
    QString name_;
};

#endif // REMOTEDOCKWIDGET_H
