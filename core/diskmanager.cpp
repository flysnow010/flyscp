#include "diskmanager.h"
#include <windows.h>
#include <winioctl.h>
#include <setupapi.h>
#include <initguid.h>
#include <newdev.h>
#include <cfgmgr32.h>
#include <regstr.h>
#include <combaseapi.h>
#include <shlobj.h>

namespace
{
    DEFINE_GUID(GUID_DEVINTERFACE_DISK,  0x53f56307L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b );
    DEFINE_GUID(GUID_DEVINTERFACE_CDROM, 0x53f56308L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b );

    long GetDiskNumber(QString const& dirverPath, bool isDevicePath = false)
    {
        long diskNumber  = -1;
        QString volumeAccessPath = isDevicePath ? dirverPath : QString("\\\\.\\%1").arg(dirverPath.toUpper());
        HANDLE hVolume = CreateFile(volumeAccessPath.toStdWString().c_str(), 0,
                   FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

        if(hVolume == INVALID_HANDLE_VALUE)
            return diskNumber;

        STORAGE_DEVICE_NUMBER sdn;
        DWORD dwBytesReturned = 0;
        long res = DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER,
                                   nullptr, 0, &sdn, sizeof(sdn), &dwBytesReturned, nullptr);
         if (res)
            diskNumber = sdn.DeviceNumber;

         CloseHandle(hVolume);
         return diskNumber;
    }

    UINT GetDriverType(QString const& dirverPath)
    {
        QString rootPath = QString("%1\\").arg(dirverPath.toUpper());
        return GetDriveType(rootPath.toStdWString().c_str());
    }

    DEVINST GetDrivesDevInstByDiskNumber(long diskNumber, long driverType)
    {
        if(driverType != DRIVE_REMOVABLE
            && driverType != DRIVE_FIXED
            && driverType != DRIVE_CDROM)
        return 0;

        const GUID* guid;
        if(driverType == DRIVE_CDROM)
            guid = &GUID_DEVINTERFACE_CDROM;
        else
            guid = &GUID_DEVINTERFACE_DISK;

        HDEVINFO hDevInfo = SetupDiGetClassDevs(guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        if (hDevInfo == INVALID_HANDLE_VALUE)
            return 0;

        DWORD dwIndex = 0;
        SP_DEVICE_INTERFACE_DATA devInterfaceData = {};
        devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        while(true)
        {
            BOOL bRet = SetupDiEnumDeviceInterfaces(hDevInfo, nullptr, guid, dwIndex, &devInterfaceData);
            if(!bRet)
                break;

            SP_DEVICE_INTERFACE_DATA spdid;
            DWORD dwSize;

            spdid.cbSize = sizeof(spdid);
            SetupDiEnumInterfaceDevice(hDevInfo, nullptr, guid, dwIndex, &spdid);
            SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, nullptr, 0, &dwSize, nullptr);

            BYTE buf[1024];
            PSP_DEVICE_INTERFACE_DETAIL_DATA pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)buf;
            if ( dwSize!=0 && dwSize<=sizeof(buf) ) {
                pspdidd->cbSize = sizeof(*pspdidd);

                SP_DEVINFO_DATA spdd;

                ZeroMemory((PVOID)&spdd, sizeof(spdd));
                spdd.cbSize = sizeof(spdd);
                long res = SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, pspdidd, dwSize, &dwSize, &spdd);
                if(res)
                {
                    if(GetDiskNumber(QString::fromStdWString(pspdidd->DevicePath), true) == diskNumber)
                    {
                         SetupDiDestroyDeviceInfoList(hDevInfo);
                         return spdd.DevInst;
                    }
                }
            }
            dwIndex++;
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return 0;
    }

    bool IsRemovable(DEVINST devInst)
    {
        ULONG status = 0;
        ULONG problemNumber = 0;
        long res = CM_Get_DevNode_Status(&status, &problemNumber, devInst, 0);
        return (res == CR_SUCCESS && (status & DN_REMOVABLE) != 0);
    }

    bool RemoveDisk(long diskNumber, long driverType, int tryCount = 3)
    {
        if(diskNumber < 0)
            return false;

        DEVINST devInst = GetDrivesDevInstByDiskNumber(diskNumber, driverType);
        if(!devInst)
            return false;
        long res  = CM_Get_Parent(&devInst, devInst, 0);// disk's parent, e.g the usb device, sata port
        if(res != CR_SUCCESS)
            return false;

        bool isRemovable = IsRemovable(devInst);
        WCHAR vetoName[MAX_PATH];
        PNP_VETO_TYPE vetoType = PNP_VetoTypeUnknown;
        for(int i = 0; i < tryCount; i++)
        {
            vetoName[0] = 0;
            if(isRemovable)
                res = CM_Request_Device_Eject(devInst, &vetoType, vetoName, sizeof(vetoName), 0);
            else
                res = CM_Query_And_Remove_SubTree(devInst, &vetoType, vetoName, sizeof(vetoName), 0);
            bool bSuccess = (res == CR_SUCCESS && vetoType == PNP_VetoTypeUnknown);
            if(bSuccess)
                return true;
            Sleep(200);
        }
        return false;
    }
}

DiskManager::DiskManager(QString const& dirverPath)
    : dirverType_(static_cast<DriverType>(GetDriverType(dirverPath)))
    , diskNumber_(GetDiskNumber(dirverPath))

{
}

bool DiskManager::removeDisk()
{
    if(dirverType_ != DRIVER_TYPE_REMOVABLE)
        return false;

    return RemoveDisk(diskNumber_, (long)dirverType_);
}

void  DiskManager::ejectDisk(QString const& dirverPath)
{
    DriverType dirverType(static_cast<DriverType>(GetDriverType(dirverPath)));
    long diskNumber(GetDiskNumber(dirverPath));

    if(dirverType != DRIVER_TYPE_REMOVABLE)
        emit finished(tr("Disk is not removeable!"));

    bool isOK =  RemoveDisk(diskNumber, (long)dirverType);
    if(isOK)
        emit finished(tr("Eject is successful!"));
    else
        emit finished(tr("Eject is fail! Disk is using!"));
}

void DiskManager::Format(QString const& driver)
{
    int diskId = driver[0].toLatin1() - 'a';
    SHFormatDrive(0, diskId, SHFMT_ID_DEFAULT, SHFMT_OPT_FULL);
}

bool DiskManager::Connect(QString const& driver)
{
    //WnetAddConnection2
    return true;
}
bool DiskManager::Disconnect(QString const& driver)
{
    //WNetCancelConnection2
    return true;
}

DiskManagerController::DiskManagerController(QObject *parent)
    : QObject(parent)
{
    DiskManager* worker = new DiskManager;
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &DiskManagerController::onEjectDisk, worker, &DiskManager::ejectDisk);
    connect(worker, &DiskManager::finished, this, &DiskManagerController::finished);
    workerThread.start();
}

DiskManagerController::~DiskManagerController()
{
    workerThread.quit();
    workerThread.wait();
}

