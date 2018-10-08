#include "dattachedudisks2device.h"
#include "diskcontrolitem.h"

#include <dfmdiskmanager.h>
#include <dfmblockdevice.h>
#include <dfmdiskdevice.h>
#include <QStorageInfo>

DFM_USE_NAMESPACE

DAttachedUdisks2Device::DAttachedUdisks2Device(const DFMBlockDevice *blockDevicePointer)
{
    mountPoint = blockDevicePointer->mountPoints().first();
    deviceDBusId = blockDevicePointer->path();
    c_blockDevice.reset(DFMDiskManager::createBlockDevice(deviceDBusId)); // not take the ownership of the passed pointer.
}

bool DAttachedUdisks2Device::detachable()
{
    QScopedPointer<DFMDiskDevice> diskDev(DFMDiskManager::createDiskDevice(blockDevice()->drive()));
    return diskDev->removable();
}

void DAttachedUdisks2Device::detach()
{
    blockDevice()->unmount({});
    if (blockDevice()->device().startsWith("/dev/sr")) { // is a DVD driver
        QScopedPointer<DFMDiskDevice> diskDev(DFMDiskManager::createDiskDevice(blockDevice()->drive()));
        if (diskDev->ejectable()) {
            diskDev->eject({});
        }
    }
}

QString DAttachedUdisks2Device::displayName()
{
    QStorageInfo storage_info(mountPoint);
    bool hasLabelName = true;
    QString result;

    if (blockDevice()->isValid()) {
        QString devName = blockDevice()->idLabel();
        if (devName.isEmpty()) {
            hasLabelName = false;
            devName = qApp->translate("DeepinStorage", "%1 Volume").arg(DiskControlItem::formatDiskSize(blockDevice()->size()));
        }

        // Deepin i10n Label text (_dde_text):
        if (devName.startsWith(ddeI18nSym)) {
            devName = devName.mid(ddeI18nSym.size(), devName.size() - ddeI18nSym.size());
            devName = qApp->translate("DeepinStorage", devName.toUtf8().constData());
        }

        result = devName;
    }

    if (storage_info.isValid()) {
        if (!hasLabelName) {
            qint64 bytesTotal = storage_info.bytesTotal();
            result = qApp->translate("DeepinStorage", "%1 Volume").arg(DiskControlItem::formatDiskSize(bytesTotal));
        }
    }

    return result;
}

bool DAttachedUdisks2Device::deviceUsageValid()
{
    QStorageInfo storage_info(mountPoint);
    return storage_info.isValid();
}

QPair<quint64, quint64> DAttachedUdisks2Device::deviceUsage()
{
    QStorageInfo storage_info(mountPoint);

    if (storage_info.isValid()) {
        qint64 bytesTotal = storage_info.bytesTotal();
        qint64 bytesFree = storage_info.bytesFree();
        return QPair<quint64, quint64>(bytesFree, bytesTotal);
    }

    return QPair<quint64, quint64>(-1, 0);
}

QString DAttachedUdisks2Device::iconName()
{
    QScopedPointer<DFMDiskDevice> diskDev(DFMDiskManager::createDiskDevice(blockDevice()->drive()));

    bool isDvd = blockDevice()->device().startsWith("/dev/sr");
    bool isRemovable = diskDev->removable();
    QString iconName = QStringLiteral("drive-harddisk");

    if (isRemovable) {
        iconName = QStringLiteral("drive-removable-media-usb");
    }

    if (isDvd) {
        iconName = QStringLiteral("media-optical");
    }

    return iconName;
}

QUrl DAttachedUdisks2Device::mountpointUrl()
{
    return QUrl::fromLocalFile(mountPoint);
}

DFMBlockDevice *DAttachedUdisks2Device::blockDevice()
{
    return c_blockDevice.data();
}
