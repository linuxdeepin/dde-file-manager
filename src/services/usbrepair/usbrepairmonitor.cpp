// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbrepairmonitor.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QTimer>
#include <QProcess>
#include <QFile>
#include <QRegularExpression>

SERVICEUSBREPAIR_USE_NAMESPACE

UsbRepairMonitor::UsbRepairMonitor(QObject *parent)
    : QObject(parent)
{
}

UsbRepairMonitor::~UsbRepairMonitor()
{
    stopMonitoring();
}

void UsbRepairMonitor::startMonitoring()
{
    if (m_udisks2ObjMgr)
        return;

    // Register DBUS meta types - required for InterfacesAdded signal
    qDBusRegisterMetaType<QVariantMap>();
    qDBusRegisterMetaType<QMap<QString, QVariantMap>>();

    QDBusConnection bus = QDBusConnection::systemBus();

    m_udisks2ObjMgr = new QDBusInterface(
        Defines::kUdisks2Service,
        Defines::kUdisks2Path,
        Defines::kUdisks2ObjectMgrIface,
        bus, this);

    bool connected1 = bus.connect(Defines::kUdisks2Service,
                                   Defines::kUdisks2Path,
                                   Defines::kUdisks2ObjectMgrIface,
                                   "InterfacesAdded",
                                   this, SLOT(onInterfacesAdded(QDBusObjectPath, QMap<QString, QVariantMap>)));
    fmInfo() << "UsbRepairMonitor: InterfacesAdded connected:" << connected1;

    bool connected2 = bus.connect(Defines::kUdisks2Service,
                                   Defines::kUdisks2Path,
                                   Defines::kUdisks2ObjectMgrIface,
                                   "InterfacesRemoved",
                                   this, SLOT(onInterfacesRemoved(QDBusObjectPath, QStringList)));
    fmInfo() << "UsbRepairMonitor: InterfacesRemoved connected:" << connected2;
    fmInfo() << "UsbRepairMonitor: started monitoring udisks2";
}

void UsbRepairMonitor::stopMonitoring()
{
    if (!m_udisks2ObjMgr)
        return;

    QDBusConnection bus = QDBusConnection::systemBus();
    bus.disconnect(Defines::kUdisks2Service,
                   Defines::kUdisks2Path,
                   Defines::kUdisks2ObjectMgrIface,
                   "InterfacesAdded",
                   this, SLOT(onInterfacesAdded(QDBusObjectPath, QMap<QString, QVariantMap>)));
    bus.disconnect(Defines::kUdisks2Service,
                   Defines::kUdisks2Path,
                   Defines::kUdisks2ObjectMgrIface,
                   "InterfacesRemoved",
                   this, SLOT(onInterfacesRemoved(QDBusObjectPath, QStringList)));

    delete m_udisks2ObjMgr;
    m_udisks2ObjMgr = nullptr;

    fmInfo() << "UsbRepairMonitor: stopped monitoring";
}

void UsbRepairMonitor::onInterfacesAdded(
    const QDBusObjectPath &objectPath,
    const QMap<QString, QVariantMap> &interfacesAndProperties)
{
    QString objPath = objectPath.path();

    // Case 1: Filesystem interface added - normal detection path
    if (interfacesAndProperties.contains(Defines::kUdisks2FilesystemIface)) {
        fmDebug() << "UsbRepairMonitor: filesystem interface added:" << objPath;

        // Delay check to let udisks2 finish auto-mount
        QTimer::singleShot(Defines::kMountSettleDelayMs, this, [this, objPath]() {
            checkDeviceHealth(objPath);
        });
        return;
    }

    // Case 2: Block interface added but no Filesystem interface
    // This usually means filesystem is too corrupted to recognize
    if (interfacesAndProperties.contains(Defines::kUdisks2BlockIface)) {
        fmDebug() << "UsbRepairMonitor: block interface added without filesystem:" << objPath;

        // Delay check to see if filesystem appears later
        QTimer::singleShot(Defines::kMountSettleDelayMs, this, [this, objPath]() {
            checkMissingFilesystem(objPath);
        });
    }
}

void UsbRepairMonitor::onInterfacesRemoved(
    const QDBusObjectPath &objectPath,
    const QStringList &interfaces)
{
    Q_UNUSED(interfaces)
    QString objPath = objectPath.path();
    emit errorCleared(objPath);
}

void UsbRepairMonitor::checkDeviceHealth(const QString &blockObjPath)
{
    QString deviceName;
    if (!isUsbDevice(blockObjPath, &deviceName)) {
        return;
    }

    QString deviceFile = getDeviceFile(blockObjPath);
    if (deviceFile.isEmpty()) {
        fmDebug() << "UsbRepairMonitor: no device file for:" << blockObjPath;
        return;
    }

    // Skip whole-disk devices (e.g., /dev/sdb), only process partitions (e.g., /dev/sdb1)
    // This prevents duplicate notifications for the same USB device
    QRegularExpression re(R"(.+[0-9]+$)");
    if (!re.match(deviceFile).hasMatch()) {
        fmDebug() << "UsbRepairMonitor: skipping whole-disk device:" << deviceFile;
        return;
    }

    QString fsType = getFsType(deviceFile);
    if (fsType.isEmpty()) {
        fmDebug() << "UsbRepairMonitor: cannot determine fs type for:" << deviceFile;

        // Notify user about unrecognized filesystem (deviceName already obtained above)
        emit errorDetected(deviceFile, deviceName, "unknown",
                           Defines::kErrorTypeMountFailed, false,
                           tr("Filesystem type cannot be detected. The device may be severely damaged "
                              "or formatted with an unsupported filesystem. Data recovery may require professional tools."));
        return;
    }

    // Check whitelist
    if (!Defines::kSupportedFsTypes.contains(fsType)) {
        fmDebug() << "UsbRepairMonitor: unsupported fs type:" << fsType;
        return;
    }

    // Check hardware read-only
    if (isHardwareReadOnly(deviceFile)) {
        emit errorDetected(deviceFile, deviceName, fsType,
                           Defines::kErrorTypeReadOnly, false,
                           tr("Device is hardware write-protected, flash memory may be failing. "
                              "Please back up data immediately."));
        return;
    }

    bool mounted = isDeviceMounted(deviceFile);

    // If device is mounted successfully after 3 seconds, it's healthy - skip dirty bit check
    if (mounted) {
        fmDebug() << "UsbRepairMonitor: device mounted successfully after delay, skipping dirty bit check:" << deviceFile;
        return;
    }

    // If mount failed (not mounted), check if filesystem damage is the cause
    if (checkDirtyBit(deviceFile, fsType)) {
        emit errorDetected(deviceFile, deviceName, fsType,
                           Defines::kErrorTypeMountFailed, true,
                           tr("Filesystem error detected, device may not have been safely ejected. "
                              "Mount might have failed due to filesystem corruption."));
    }
}

bool UsbRepairMonitor::isUsbDevice(const QString &blockObjPath, QString *deviceName)
{
    QDBusInterface blockIface(
        Defines::kUdisks2Service,
        blockObjPath,
        Defines::kUdisks2BlockIface,
        QDBusConnection::systemBus());

    if (!blockIface.isValid())
        return false;

    // Get drive object path
    QDBusObjectPath drivePath = blockIface.property("Drive").value<QDBusObjectPath>();
    if (drivePath.path().isEmpty() || drivePath.path() == "/")
        return false;

    // Check drive ConnectionBus == "usb"
    QDBusInterface driveIface(
        Defines::kUdisks2Service,
        drivePath.path(),
        Defines::kUdisks2DriveIface,
        QDBusConnection::systemBus());

    if (!driveIface.isValid())
        return false;

    QString connectionBus = driveIface.property("ConnectionBus").toString();
    if (connectionBus != "usb")
        return false;

    if (deviceName) {
        *deviceName = driveIface.property("Id").toString();
        QString vendor = driveIface.property("Vendor").toString();
        QString model = driveIface.property("Model").toString();
        if (!vendor.isEmpty() || !model.isEmpty()) {
            *deviceName = (vendor.isEmpty() ? "" : vendor + " ")
                    + (model.isEmpty() ? "" : model);
        }
    }

    // Check removable
    bool removable = driveIface.property("Removable").toBool();
    return removable;
}

QString UsbRepairMonitor::getDeviceFile(const QString &blockObjPath)
{
    QDBusInterface blockIface(
        Defines::kUdisks2Service,
        blockObjPath,
        Defines::kUdisks2BlockIface,
        QDBusConnection::systemBus());

    if (!blockIface.isValid())
        return {};

    // Device 属性是 D-Bus 字节数组(ay)，常以 '\0' 结尾；直接 toString() 会保留尾部空字节，
    // 导致后续路径匹配、进程参数出错。用 constData() 截断尾部空字节。
    QByteArray dev = blockIface.property("Device").toByteArray();
    return QString::fromUtf8(dev.constData());
}

QString UsbRepairMonitor::getFsType(const QString &deviceFile)
{
    QProcess proc;
    proc.start("blkid", { "-o", "value", "-s", "TYPE", deviceFile });
    proc.waitForFinished(5000);
    QString output = proc.readAllStandardOutput().trimmed();
    fmInfo() << "UsbRepairMonitor: command: blkid -o value -s TYPE" << deviceFile << "| exitCode:" << proc.exitCode() << "| output:" << output;
    if (proc.exitCode() == 0) {
        if (!output.isEmpty()) {
            // ext2/ext3 use the same e2fsck tool as ext4; normalize so the
            // whitelist and monitoring logic treat them uniformly.
            if (output == "ext2" || output == "ext3")
                output = "ext4";
            return output;
        }
    }

    // Fallback: try udisks2 property
    // (This is less accurate but serves as backup)
    return {};
}

bool UsbRepairMonitor::isDeviceMounted(const QString &deviceFile)
{
    // Read /proc/mounts directly — most reliable
    QFile mounts("/proc/mounts");
    if (mounts.open(QIODevice::ReadOnly)) {
        QByteArray data = mounts.readAll();
        mounts.close();
        for (const QByteArray &line : data.split('\n')) {
            if (line.startsWith(deviceFile.toUtf8() + " "))
                return true;
        }
    }
    return false;
}

bool UsbRepairMonitor::isHardwareReadOnly(const QString &deviceFile)
{
    // Get the whole-disk device (strip partition number)
    // e.g. /dev/sdb1 → /dev/sdb
    QString wholeDisk = deviceFile;
    QRegularExpression re(R"((.+)[0-9]+$)");
    QRegularExpressionMatch match = re.match(deviceFile);
    if (match.hasMatch())
        wholeDisk = match.captured(1);

    QProcess proc;
    proc.start("blockdev", { "--getro", wholeDisk });
    proc.waitForFinished(3000);
    QString output = proc.readAllStandardOutput().trimmed();
    fmInfo() << "UsbRepairMonitor: command: blockdev --getro" << wholeDisk << "| exitCode:" << proc.exitCode() << "| output:" << output;
    if (proc.exitCode() == 0) {
        return output == "1";
    }
    return false;
}

bool UsbRepairMonitor::checkDirtyBit(const QString &deviceFile, const QString &fsType)
{
    QProcess proc;
    QString command;

    if (fsType == "vfat") {
        command = "fsck.fat -n " + deviceFile;
        proc.start("fsck.fat", { "-n", deviceFile });
    } else if (fsType == "exfat") {
        command = "fsck.exfat -n " + deviceFile;
        proc.start("fsck.exfat", { "-n", deviceFile });
    } else if (fsType == "ntfs") {
        command = "ntfsfix -n " + deviceFile;
        proc.start("ntfsfix", { "-n", deviceFile });
    } else if (fsType == "ext4") {
        command = "e2fsck -n " + deviceFile;
        proc.start("e2fsck", { "-n", deviceFile });
    } else {
        return false;
    }

    proc.waitForFinished(30000);
    int exitCode = proc.exitCode();
    QString output = QString::fromUtf8(proc.readAllStandardOutput())
            + QString::fromUtf8(proc.readAllStandardError());
    fmInfo() << "UsbRepairMonitor: command:" << command << "| exitCode:" << exitCode << "| output:" << output;

    // exitCode == 0 means clean
    // exitCode > 0 means errors detected
    // Negative / crash means tool not available
    if (exitCode < 0)
        return false;

    return exitCode > 0;
}

void UsbRepairMonitor::checkMissingFilesystem(const QString &blockObjPath)
{
    QString deviceName;
    if (!isUsbDevice(blockObjPath, &deviceName)) {
        return;
    }

    QString deviceFile = getDeviceFile(blockObjPath);
    if (deviceFile.isEmpty()) {
        fmDebug() << "UsbRepairMonitor: no device file for:" << blockObjPath;
        return;
    }

    // Skip whole-disk devices (e.g., /dev/sdb), only process partitions (e.g., /dev/sdb1)
    // This prevents duplicate notifications for the same USB device
    QRegularExpression re(R"(.+[0-9]+$)");
    if (!re.match(deviceFile).hasMatch()) {
        fmDebug() << "UsbRepairMonitor: skipping whole-disk device:" << deviceFile;
        return;
    }

    // Double-check if filesystem has appeared in the meantime.
    // 直接查询 org.freedesktop.UDisks2.Filesystem 接口的 MountPoints 属性：
    // Properties.Get 对不存在的接口返回无效 QVariant，符合 D-Bus 规范。
    QDBusInterface fsIface(
        Defines::kUdisks2Service,
        blockObjPath,
        Defines::kUdisks2FilesystemIface,
        QDBusConnection::systemBus());

    if (fsIface.isValid() && fsIface.property("MountPoints").isValid()) {
        fmDebug() << "UsbRepairMonitor: filesystem appeared for:" << deviceFile;
        checkDeviceHealth(blockObjPath);
        return;
    }

    // Still no filesystem interface - try to detect using 'file' command
    // This can identify filesystem type even when severely corrupted
    QProcess proc;
    proc.start("file", { "-s", deviceFile });
    proc.waitForFinished(5000);
    QString output = proc.readAllStandardOutput();
    fmInfo() << "UsbRepairMonitor: command: file -s" << deviceFile << "| exitCode:" << proc.exitCode() << "| output:" << output;

    if (proc.exitCode() != 0) {
        fmDebug() << "UsbRepairMonitor: file command failed for:" << deviceFile;
        return;
    }

    QString fsType;

    // Parse file command output for filesystem signatures
    if (output.contains("FAT", Qt::CaseInsensitive))
        fsType = "vfat";
    else if (output.contains("exFAT", Qt::CaseInsensitive))
        fsType = "exfat";
    else if (output.contains("NTFS", Qt::CaseInsensitive))
        fsType = "ntfs";
    else if (output.contains("ext2", Qt::CaseInsensitive) ||
             output.contains("ext3", Qt::CaseInsensitive) ||
             output.contains("ext4", Qt::CaseInsensitive))
        fsType = "ext4";

    if (fsType.isEmpty()) {
        fmDebug() << "UsbRepairMonitor: cannot detect filesystem type for:" << deviceFile;

        // Notify user about unrecognized filesystem
        emit errorDetected(deviceFile, deviceName, "unknown",
                           Defines::kErrorTypeMountFailed, false,
                           tr("Filesystem type cannot be detected. The device may be severely damaged "
                              "or formatted with an unsupported filesystem. Data recovery may require professional tools."));
        return;
    }

    // Check whitelist
    if (!Defines::kSupportedFsTypes.contains(fsType)) {
        fmDebug() << "UsbRepairMonitor: unsupported fs type:" << fsType;
        return;
    }

    // Check hardware read-only
    if (isHardwareReadOnly(deviceFile)) {
        emit errorDetected(deviceFile, deviceName, fsType,
                           Defines::kErrorTypeReadOnly, false,
                           tr("Device is hardware write-protected, flash memory may be failing. "
                              "Please back up data immediately."));
        return;
    }

    // Found a supported but corrupted filesystem
    emit errorDetected(deviceFile, deviceName, fsType,
                       Defines::kErrorTypeMountFailed, true,
                       tr("Filesystem is severely corrupted and cannot be recognized. "
                          "Data may be recoverable through repair."));
}

