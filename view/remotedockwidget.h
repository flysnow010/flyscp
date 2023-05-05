#ifndef REMOTEDOCKWIDGET_H
#define REMOTEDOCKWIDGET_H
#include <core/sshsettings.h>
#include <QDockWidget>

namespace Ui {
class RemoteDockWidget;
}
class RemoteDirModel;
class SFtpSession;
class RemoteDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit RemoteDockWidget(QWidget *parent = nullptr);
    ~RemoteDockWidget();

    void start(SSHSettings const& settings);
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
    void del();
    void rename();
    void copyFilepath();
    void properties();
    void permissions();
private:
    Ui::RemoteDockWidget *ui;
    RemoteDirModel* model_;
    SFtpSession* sftp;
};

#endif // REMOTEDOCKWIDGET_H
