#ifndef DISKMANAGER_H
#define DISKMANAGER_H
#include <QString>

class DiskManager
{
public:
    DiskManager(QString const& dirverPath);

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

private:
    DriverType dirverType_ = DRIVER_TYPE_UNKNOWN;
    long diskNumber_ = -1;
};

#endif // DISKMANAGER_H
