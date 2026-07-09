// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbrepairworker.h"

#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QFile>
#include <QTimer>
#include <QProcess>
#include <QRegularExpression>

#include <polkit-qt6-1/PolkitQt1/Authority>
#include <errno.h>

SERVICEUSBREPAIR_USE_NAMESPACE

UsbRepairWorker::UsbRepairWorker(QObject *parent)
    : QObject(parent)
{
}

UsbRepairWorker::~UsbRepairWorker()
{

    if (m_timeoutTimer)
        m_timeoutTimer->stop();
    if (m_currentProcess) {
        m_currentProcess->kill();
        m_currentProcess->waitForFinished(5000);
    }
}

bool UsbRepairWorker::startRepair(const QString &devicePath, const QString &callerBusName,
                                  QString &errorMessage)
{
    // 1. Validate device path
    if (!validateDevicePath(devicePath)) {
        errorMessage = tr("Invalid device path: %1").arg(devicePath);
        return false;
    }

    // 2. Idempotency check
    if (isRepairing(devicePath)) {
        errorMessage = tr("Device is already being repaired: %1").arg(devicePath);
        return false;
    }

    // 3. Detect filesystem type
    QString fsType = detectFsType(devicePath);
    if (fsType.isEmpty()) {
        errorMessage = tr("Cannot detect filesystem type for: %1").arg(devicePath);
        return false;
    }

    // 4. Whitelist check
    if (!Defines::kSupportedFsTypes.contains(fsType)) {
        errorMessage = tr("Unsupported filesystem type: %1").arg(fsType);
        return false;
    }

    // 5. Check hardware read-only
    QProcess blockdevProc;
    QString wholeDisk = devicePath;
    QRegularExpression re(R"((.+)[0-9]+$)");
    QRegularExpressionMatch match = re.match(devicePath);
    if (match.hasMatch())
        wholeDisk = match.captured(1);

    blockdevProc.start("blockdev", { "--getro", wholeDisk });
    blockdevProc.waitForFinished(3000);
    if (blockdevProc.exitCode() == 0 && blockdevProc.readAllStandardOutput().trimmed() == "1") {
        errorMessage = tr("Device is hardware write-protected, cannot repair");
        return false;
    }

    // 6. PolKit authorization
    if (!checkAuthorization(callerBusName)) {
        errorMessage = tr("Authorization failed");
        return false;
    }

    // 7. Umount if mounted
    bool wasMounted = false;
    QFile mounts("/proc/mounts");
    if (mounts.open(QIODevice::ReadOnly)) {
        QByteArray data = mounts.readAll();
        mounts.close();
        for (const QByteArray &line : data.split('\n')) {
            if (line.startsWith(devicePath.toUtf8() + " ")) {
                wasMounted = true;
                break;
            }
        }
    }

    if (wasMounted && !umountDevice(devicePath)) {
        errorMessage = tr("Failed to unmount device: %1").arg(devicePath);
        return false;
    }

    // 8. Start repair
    m_currentDevice = devicePath;
    m_currentFsType = fsType;
    m_activeRepairs.insert(devicePath);
    executeFsck(devicePath, fsType);

    return true;
}

bool UsbRepairWorker::cancelRepair(const QString &devicePath, const QString &callerBusName)
{
    if (!checkAuthorization(callerBusName))
        return false;

    if (m_currentDevice != devicePath || !m_currentProcess)
        return false;

    m_currentProcess->kill();
    return true;
}

bool UsbRepairWorker::isRepairing(const QString &devicePath) const
{
    return m_activeRepairs.contains(devicePath);
}

void UsbRepairWorker::onFsckReadyRead()
{
    if (!m_currentProcess)
        return;

    // Read both stdout and stderr and accumulate
    QString stdout = QString::fromUtf8(m_currentProcess->readAllStandardOutput());
    QString stderr = QString::fromUtf8(m_currentProcess->readAllStandardError());
    QString output = stdout + stderr;

    // Accumulate for final logging
    m_fsckOutput += output;

    for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
        // Parse progress percentage from output
        int percent = -1;   // -1 = indeterminate
        QRegularExpression percentRe(R"((\d+)%)");
        QRegularExpressionMatch match = percentRe.match(line);
        if (match.hasMatch())
            percent = match.captured(1).toInt();

        emit progress(m_currentDevice, percent, line);
    }
}

void UsbRepairWorker::onFsckFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)

    // 防御：异常路径下可能重入或 process 已被销毁
    if (!m_currentProcess)
        return;

    // 任务结束，停止超时定时器，避免残留触发误杀后续任务的进程
    if (m_timeoutTimer)
        m_timeoutTimer->stop();

    QString device = m_currentDevice;
    QString fsType = m_currentFsType;
    QString summary;
    bool success = false;

    // Read any remaining output that wasn't captured by onFsckReadyRead
    QString remainingOutput = QString::fromUtf8(m_currentProcess->readAllStandardError())
            + QString::fromUtf8(m_currentProcess->readAllStandardOutput());
    m_fsckOutput += remainingOutput;

    fmInfo() << "UsbRepairWorker: fsck finished | device:" << device << "| fstype:" << fsType << "| exitCode:" << exitCode << "| timedOut:" << m_timedOut << "| output:" << m_fsckOutput;

    if (m_timedOut) {
        // kill() 触发的终止：统一在此收尾，不再由 onFsckTimeout 自行 emit
        success = false;
        summary = tr("Repair timed out after %1 seconds")
                .arg(Defines::kFsckTimeoutMs / 1000);
    } else if (exitCode == 0) {
        success = true;
        summary = tr("Filesystem repair completed successfully");
    } else if (fsType == "ntfs" && exitCode != 0) {
        success = false;
        summary = tr("NTFS repair capability is limited on Linux. "
                     "Please connect the device to a Windows system for deep repair.");
    } else {
        success = (exitCode == 1);   // exit code 1 = errors corrected
        if (success) {
            summary = tr("Filesystem errors have been repaired");
        } else {
            summary = tr("Filesystem repair failed (exit code: %1)").arg(exitCode);
            if (!m_fsckOutput.trimmed().isEmpty())
                summary += "\n" + m_fsckOutput.trimmed();
        }
    }

    m_timedOut = false;
    m_activeRepairs.remove(device);
    m_currentDevice.clear();
    m_currentFsType.clear();
    m_currentProcess->deleteLater();
    m_currentProcess = nullptr;

    emit finished(device, success, summary);
}

void UsbRepairWorker::onFsckTimeout()
{
    if (!m_currentProcess)
        return;

    // 仅置标志并 kill，finished 信号统一由 onFsckFinished 发射一次，
    // 避免 kill() 异步触发 QProcess::finished 造成重复 emit。
    m_timedOut = true;
    m_currentProcess->kill();
}

void UsbRepairWorker::onFsckErrorOccurred(QProcess::ProcessError error)
{
    if (error != QProcess::FailedToStart)
        return;

    // 启动失败时 QProcess 不会发射 finished，需在此统一收尾，避免状态悬挂。
    if (!m_currentProcess)
        return;

    if (m_timeoutTimer)
        m_timeoutTimer->stop();

    const QString device = m_currentDevice;
    const QString summary = tr("Failed to start repair process: %1")
            .arg(m_currentProcess->errorString().isEmpty()
                     ? tr("unknown error")
                     : m_currentProcess->errorString());

    m_activeRepairs.remove(device);
    m_currentDevice.clear();
    m_currentFsType.clear();
    m_currentProcess->deleteLater();
    m_currentProcess = nullptr;

    fmWarning() << "UsbRepairWorker: fsck failed to start | device:" << device
                << "| error:" << static_cast<int>(error);
    emit finished(device, false, summary);
}

bool UsbRepairWorker::checkAuthorization(const QString &callerBusName)
{
    using namespace PolkitQt1;

    if (callerBusName.isEmpty())
        return false;

    Authority::Result result = Authority::instance()->checkAuthorizationSync(
        Defines::kPolkitActionId,
        SystemBusNameSubject(callerBusName),
        Authority::AllowUserInteraction);

    return result == Authority::Yes;
}

bool UsbRepairWorker::umountDevice(const QString &devicePath)
{
    QProcess proc;
    proc.start("umount", { devicePath });
    proc.waitForFinished(10000);
    if (proc.exitCode() != 0) {
        fmWarning() << "UsbRepairWorker: umount failed:" << proc.readAllStandardError();
        return false;
    }
    return true;
}

QString UsbRepairWorker::detectFsType(const QString &devicePath)
{
    QProcess proc;

    // Method 1: blkid (fastest, works for normal cases)
    proc.start("blkid", { "-o", "value", "-s", "TYPE", devicePath });
    proc.waitForFinished(5000);
    if (proc.exitCode() == 0) {
        QString output = proc.readAllStandardOutput().trimmed();
        if (!output.isEmpty()) {
            // ext2/ext3 use the same e2fsck tool as ext4; normalize so the
            // whitelist and repair logic treat them uniformly.
            if (output == "ext2" || output == "ext3")
                output = "ext4";
            return output;
        }
    }

    // Method 2: file command (fallback for corrupted filesystems)
    // Reads magic numbers and raw structures even when filesystem is severely damaged
    proc.start("file", { "-s", devicePath });
    proc.waitForFinished(5000);
    QString fileOutput = proc.readAllStandardOutput();
    fmInfo() << "UsbRepairWorker: command: file -s" << devicePath << "| exitCode:" << proc.exitCode() << "| output:" << fileOutput;
    if (proc.exitCode() == 0) {
        // Parse file command output for filesystem signatures
        if (fileOutput.contains("FAT", Qt::CaseInsensitive))
            return "vfat";
        else if (fileOutput.contains("exFAT", Qt::CaseInsensitive))
            return "exfat";
        else if (fileOutput.contains("NTFS", Qt::CaseInsensitive))
            return "ntfs";
        else if (fileOutput.contains("ext2 filesystem", Qt::CaseInsensitive) ||
                 fileOutput.contains("ext3 filesystem", Qt::CaseInsensitive) ||
                 fileOutput.contains("ext4 filesystem", Qt::CaseInsensitive))
            return "ext4";
    }

    return {};
}

bool UsbRepairWorker::validateDevicePath(const QString &devicePath)
{
    if (devicePath.isEmpty())
        return false;
    if (!devicePath.startsWith("/dev/"))
        return false;
    // Must not contain path traversal
    if (devicePath.contains(".."))
        return false;
    if (!QFile::exists(devicePath))
        return false;

    // 安全：仅允许修复 USB 可移动设备，拒绝系统盘等固定磁盘被卸载/修复
    return isUsbBlockDevice(devicePath);
}

bool UsbRepairWorker::isUsbBlockDevice(const QString &devicePath)
{
    // 通过 udisks2 校验：block.Drive → drive.ConnectionBus=="usb" && drive.Removable
    QDBusInterface blockIface(
        Defines::kUdisks2Service,
        blockObjPathFromDevice(devicePath),
        Defines::kUdisks2BlockIface,
        QDBusConnection::systemBus());
    if (!blockIface.isValid())
        return false;

    QDBusObjectPath drivePath = blockIface.property("Drive").value<QDBusObjectPath>();
    if (drivePath.path().isEmpty() || drivePath.path() == "/")
        return false;

    QDBusInterface driveIface(
        Defines::kUdisks2Service,
        drivePath.path(),
        Defines::kUdisks2DriveIface,
        QDBusConnection::systemBus());
    if (!driveIface.isValid())
        return false;

    if (driveIface.property("ConnectionBus").toString() != "usb")
        return false;

    return driveIface.property("Removable").toBool();
}

QString UsbRepairWorker::blockObjPathFromDevice(const QString &devicePath)
{
    // Convert /dev/sdb1 to /org/freedesktop/UDisks2/block_devices/sdb1
    QString basename = devicePath.mid(5);  // Remove "/dev/"
    return QString("/org/freedesktop/UDisks2/block_devices/%1").arg(basename);
}

void UsbRepairWorker::executeFsck(const QString &devicePath, const QString &fsType)
{
    m_timedOut = false;
    m_currentProcess = new QProcess(this);
    connect(m_currentProcess, &QProcess::readyReadStandardOutput,
            this, &UsbRepairWorker::onFsckReadyRead);
    connect(m_currentProcess, &QProcess::readyReadStandardError,
            this, &UsbRepairWorker::onFsckReadyRead);
    connect(m_currentProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &UsbRepairWorker::onFsckFinished);
    connect(m_currentProcess, &QProcess::errorOccurred,
            this, &UsbRepairWorker::onFsckErrorOccurred);

    // Reset output accumulator
    m_fsckOutput.clear();

    // 超时保护：成员定时器统一管理，onFsckFinished 收尾时 stop()，
    // 避免旧任务定时器残留触发 onFsckTimeout 误杀下一个任务的进程。
    if (!m_timeoutTimer) {
        m_timeoutTimer = new QTimer(this);
        m_timeoutTimer->setSingleShot(true);
        connect(m_timeoutTimer, &QTimer::timeout, this, &UsbRepairWorker::onFsckTimeout);
    }
    m_timeoutTimer->start(Defines::kFsckTimeoutMs);

    if (fsType == "vfat") {
        m_currentProcess->start("fsck.fat", { "-a", "-w", devicePath });
    } else if (fsType == "exfat") {
        m_currentProcess->start("fsck.exfat", { "-p", devicePath });
    } else if (fsType == "ntfs") {
        m_currentProcess->start("ntfsfix", { devicePath });
    } else if (fsType == "ext4") {
        m_currentProcess->start("e2fsck", { "-p", devicePath });
    } else {
        // Unsupported type should have been filtered during preparation.
        m_timeoutTimer->stop();
        m_currentProcess->deleteLater();
        m_currentProcess = nullptr;
        m_activeRepairs.remove(devicePath);
        m_currentDevice.clear();
        emit finished(devicePath, false, tr("Unsupported filesystem type: %1").arg(fsType));
        return;
    }
    fmInfo() << "UsbRepairWorker: command:" << m_currentProcess->program()
             << m_currentProcess->arguments();
}
