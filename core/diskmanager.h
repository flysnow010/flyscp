#ifndef DISKMANAGER_H
#define DISKMANAGER_H
#include <QString>
#include <QObject>
#include <QThread>

class DiskManager : public QObject
{
    Q_OBJECT
public:
    DiskManager(QString const& dirverPath);
    DiskManager() = default;

    enum DriverType {
        DRIVER_TYPE_UNKNOWN     = 0,
        DRIVER_TYPE_NO_ROOT_DIR = 1,
        DRIVER_TYPE_REMOVABLE   = 2,
        DRIVER_TYPE_FIXED       = 3,
        DRIVER_TYPE_REMOTE      = 4,
        DRIVER_TYPE_CDROM       = 5,
        DRIVER_TYPE_RAMDISK     = 6
    };

    static void Format(QString const& driver);
    static bool Connect(QString const& driver);
    static bool Disconnect(QString const& driver);

    DriverType dirverType() const { return dirverType_; }
    bool isRemovable() const { return dirverType_ == DRIVER_TYPE_REMOVABLE; }

    bool removeDisk();
public slots:
    void ejectDisk(QString const& dirverPath);
signals:
    void finished(const QString &result);
private:
    DriverType dirverType_ = DRIVER_TYPE_UNKNOWN;
    long diskNumber_ = -1;
};

class DiskManagerController : public QObject
{
    Q_OBJECT
public:
    DiskManagerController(QObject* parent);
    ~DiskManagerController();

    void ejectDisk(QString const& dirverPath) { emit onEjectDisk(dirverPath); }
signals:
    void finished(const QString &result);
    void onEjectDisk(QString const& dirverPath);

private:
    QThread workerThread;
};

#endif // DISKMANAGER_H
