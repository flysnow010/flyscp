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
    void connected();
    void unconnected();
    void connectionError(QString const& error);
private:
    Ui::RemoteDockWidget *ui;
    RemoteDirModel* model_;
    SFtpSession* sftp;
};

#endif // REMOTEDOCKWIDGET_H
