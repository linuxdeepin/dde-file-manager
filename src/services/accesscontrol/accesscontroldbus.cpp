// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accesscontroldbus.h"
#include "accesscontroladaptor.h"
#include "utils.h"
#include "polkit/policykithelper.h"

#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/fileutils.h>

#include <QDebug>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusVariant>
#include <QtConcurrent>
#include <QDataStream>

#include <dfm-mount/dblockmonitor.h>
#include <dfm-mount/dblockdevice.h>
#include <dfm-mount/base/dmountutils.h>

#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

namespace service_accesscontrol {
DFM_LOG_REGISTER_CATEGORY(SERVICEACCESSCONTROL_NAMESPACE)
}

DFMBASE_USE_NAMESPACE
SERVICEACCESSCONTROL_USE_NAMESPACE

using ServiceCommon::PolicyKitHelper;

/*!
 * \class AccessControlDBus
 * \brief D-Bus interface class, Don't nedd a namespace
 */

static constexpr char kAccessControlManagerObjPath[] { "/org/deepin/Filemanager/AccessControlManager" };
static constexpr char kPolicyKitDiskPwdActionId[] { "org.deepin.Filemanager.AccessControlManager.DiskPwd" };
static constexpr char kPolicyKitChmodActionId[] { "org.deepin.Filemanager.AccessControlManager.Chmod" };

AccessControlDBus::AccessControlDBus(const char *name, QObject *parent)
    : QObject(parent), QDBusContext()
{
    Utils::loadDevPolicy(&globalDevPolicies);
    Utils::loadVaultPolicy(&globalVaultHidePolicies);

    errMsg.insert(kNoError, "");
    errMsg.insert(kInvalidArgs, tr("Invalid args"));
    errMsg.insert(kInvalidInvoker, tr("Invalid invoker"));

    initConnect();

    adaptor = new AccessControlManagerAdaptor(this);
    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;

    QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString(name)).registerObject(kAccessControlManagerObjPath, this, opts);
}

AccessControlDBus::~AccessControlDBus()
{
}

void AccessControlDBus::ChangeDiskPassword(const QDBusUnixFileDescriptor &credentialsFd)
{
    if (!checkAuthentication(kPolicyKitDiskPwdActionId)) {
        fmWarning() << "[AccessControlDBus::ChangeDiskPassword] Authentication failed for disk password change";
        emit DiskPasswordChecked(kAuthenticationFailed);
        return;
    }

    // Validate file descriptor
    if (!credentialsFd.isValid()) {
        fmWarning() << "[AccessControlDBus::ChangeDiskPassword] Invalid file descriptor provided";
        emit DiskPasswordChecked(kInvalidArgs);
        return;
    }

    int fd = credentialsFd.fileDescriptor();
    if (fd < 0) {
        fmWarning() << "[AccessControlDBus::ChangeDiskPassword] Invalid file descriptor value:" << fd;
        emit DiskPasswordChecked(kInvalidArgs);
        return;
    }

    // Read all data from pipe into buffer
    QByteArray buffer;
    char readBuffer[1024];
    ssize_t bytesRead;

    while ((bytesRead = read(fd, readBuffer, sizeof(readBuffer))) > 0) {
        buffer.append(readBuffer, bytesRead);
    }

    if (buffer.isEmpty()) {
        fmWarning() << "[AccessControlDBus::ChangeDiskPassword] No data received from pipe";
        emit DiskPasswordChecked(kInvalidArgs);
        return;
    }

    // Parse credentials using QDataStream
    QDataStream stream(&buffer, QIODevice::ReadOnly);
    QString oldPwd, newPwd;
    stream >> oldPwd >> newPwd;

    if (stream.status() != QDataStream::Ok) {
        fmWarning() << "[AccessControlDBus::ChangeDiskPassword] Failed to parse credentials from pipe data, stream status:" << stream.status();
        emit DiskPasswordChecked(kInvalidArgs);
        return;
    }

    const auto &devList = DeviceUtils::encryptedDisks();
    if (devList.isEmpty()) {
        fmInfo() << "[AccessControlDBus::ChangeDiskPassword] No encrypted disks found, operation completed";
        emit DiskPasswordChecked(kNoError);
        QTimer::singleShot(500, [this] { emit DiskPasswordChanged(kAccessDiskFailed); });
        return;
    }

    fmInfo() << "[AccessControlDBus::ChangeDiskPassword] Starting password change for" << devList.size() << "encrypted disks";

    QString oldPwdDec = FileUtils::decryptString(oldPwd);
    QString newPwdDec = FileUtils::decryptString(newPwd);

    const QByteArray &tmpOldPwd = oldPwdDec.toLocal8Bit();
    const QByteArray &tmpNewPwd = newPwdDec.toLocal8Bit();

    int ret = kNoError;
    QStringList successList;
    for (int i = 0; i < devList.size(); ++i) {
        struct crypt_device *cd = nullptr;
        ret = Utils::checkDiskPassword(&cd, tmpOldPwd.data(), devList[i].toLocal8Bit().data());

        if (ret == kPasswordWrong && i == 0) {
            fmWarning() << "[AccessControlDBus::ChangeDiskPassword] Password verification failed for first device:" << devList[i];
            emit DiskPasswordChecked(kPasswordWrong);
            return;
        } else if (ret == kPasswordWrong) {
            fmWarning() << "[AccessControlDBus::ChangeDiskPassword] Password inconsistency detected at device:" << devList[i];
            ret = kPasswordInconsistent;
            break;
        } else if (ret == kNoError) {
            if (i == 0) {
                fmInfo() << "[AccessControlDBus::ChangeDiskPassword] Password verification successful for first device";
                emit DiskPasswordChecked(kNoError);
            }

            ret = Utils::changeDiskPassword(cd, tmpOldPwd.data(), tmpNewPwd.data());
            if (ret == kNoError) {
                fmInfo() << "[AccessControlDBus::ChangeDiskPassword] Password changed successfully for device:" << devList[i];
            } else {
                fmCritical() << "[AccessControlDBus::ChangeDiskPassword] Failed to change password for device:" << devList[i] << "error code:" << ret;
            }
        } else {
            fmCritical() << "[AccessControlDBus::ChangeDiskPassword] Failed to check password for device:" << devList[i] << "error code:" << ret;
            break;
        }

        if (ret != kNoError)
            break;

        successList << devList[i];
    }

    // restore password
    if (ret != kNoError && !successList.isEmpty()) {
        fmWarning() << "[AccessControlDBus::ChangeDiskPassword] Rolling back password changes for" << successList.size() << "devices due to error";
        for (const auto &device : successList) {
            struct crypt_device *cd = nullptr;
            Utils::checkDiskPassword(&cd, tmpNewPwd.data(), device.toLocal8Bit().data());
            Utils::changeDiskPassword(cd, tmpNewPwd.data(), tmpOldPwd.data());
        }
    }

    if (ret == kNoError) {
        fmInfo() << "[AccessControlDBus::ChangeDiskPassword] Password change completed successfully for all devices";
    } else {
        fmCritical() << "[AccessControlDBus::ChangeDiskPassword] Password change operation failed with error code:" << ret;
    }

    emit DiskPasswordChanged(ret);
}

bool AccessControlDBus::Chmod(const QString &path, uint mode)
{
    if (!checkAuthentication(kPolicyKitChmodActionId)) {
        fmWarning() << "[AccessControlDBus::Chmod] Authentication failed for path:" << path;
        return false;
    }

    if (path.isEmpty()) {
        fmWarning() << "[AccessControlDBus::Chmod] Empty path provided";
        return false;
    }

    QFile f(path);
    if (!f.exists()) {
        fmWarning() << "[AccessControlDBus::Chmod] File does not exist:" << path;
        return false;
    }

    fmInfo() << "[AccessControlDBus::Chmod] Changing access permission for path:" << path << "to mode:" << QString::number(mode, 8);
    int ret = ::Utils::setFileMode(path.toStdString().c_str(), mode);
    if (ret != 0) {
        fmCritical() << "[AccessControlDBus::Chmod] Failed to change permission for path:" << path << "error:" << strerror(errno);
        return false;
    }
    fmInfo() << "[AccessControlDBus::Chmod] Access permission changed successfully for path:" << path;
    return true;
}

void AccessControlDBus::onBlockDevAdded(const QString &deviceId)
{
    DFM_MOUNT_USE_NS
    auto dev = monitor->createDeviceById(deviceId).objectCast<DBlockDevice>();
    if (!dev) {
        fmWarning() << "[AccessControlDBus::onBlockDevAdded] Cannot create device handler for device ID:" << deviceId;
        return;
    }

    bool canPowerOff { dev->canPowerOff() };
    QString connectionDBus { dev->getProperty(Property::kDriveConnectionBus).toString() };

    if (!canPowerOff || connectionDBus != "usb")   // 不能断电的通常为内置光驱
        return;

    bool opticalDrive { dev->mediaCompatibility().join(" ").contains("optical") };
    if (!opticalDrive)
        return;
    if (!globalDevPolicies.contains(kTypeOptical))
        return;

    int policy = globalDevPolicies.value(kTypeOptical).second;

    if (policy == kPolicyDisable) {
        fmInfo() << "[AccessControlDBus::onBlockDevAdded] Applying disable policy for optical device:" << deviceId;
        QtConcurrent::run([deviceId, dev]() {
            int retry = 5;
            while (retry-- && !dev->powerOff()) {
                fmWarning() << "[AccessControlDBus::onBlockDevAdded] Failed to power off optical device:" << deviceId << "error:" << dev->lastError().message << "retries left:" << retry;
                QThread::msleep(500);
            }
            if (retry < 0) {
                fmCritical() << "[AccessControlDBus::onBlockDevAdded] Failed to power off optical device after all retries:" << deviceId;
            } else {
                fmInfo() << "[AccessControlDBus::onBlockDevAdded] Successfully powered off optical device:" << deviceId;
            }
        });
    }
}

void AccessControlDBus::onBlockDevMounted(const QString &deviceId, const QString &mountPoint)
{
    DFM_MOUNT_USE_NS
    auto dev = monitor->createDeviceById(deviceId).objectCast<DBlockDevice>();
    if (!dev || dev->hintSystem()) {
        fmWarning() << "[AccessControlDBus::onBlockDevMounted] Cannot create device handler or device is system disk, device ID:" << deviceId;
        return;
    }

    fmInfo() << "[AccessControlDBus::onBlockDevMounted] Processing mounted device:" << deviceId << "at mount point:" << mountPoint;

    if (globalDevPolicies.contains(kTypeBlock)) {
        QString devDesc { dev->device() };
        int mode = ::Utils::accessMode(mountPoint);
        QString source = globalDevPolicies.value(kTypeBlock).first;
        int policy = globalDevPolicies.value(kTypeBlock).second;
        QString fs { dev->fileSystem() };
        
        fmInfo() << "[AccessControlDBus::onBlockDevMounted] Current access mode:" << mode << "policy:" << policy << "for device:" << devDesc;
        
        if (mode != policy) {
            if (policy == kPolicyDisable) {
                fmInfo() << "[AccessControlDBus::onBlockDevMounted] Device should be disabled, unmounting:" << devDesc;
                // unmount
            } else {
                fmInfo() << "[AccessControlDBus::onBlockDevMounted] Remounting device with policy:" << policy << "from source:" << source;
                // remount
                QtConcurrent::run([devDesc, mountPoint, fs, policy, source]() {
                    int ret = ::mount(devDesc.toLocal8Bit().data(),
                                      mountPoint.toLocal8Bit().data(),
                                      fs.toLocal8Bit().data(),
                                      MS_REMOUNT | (policy == kPolicyRonly ? MS_RDONLY : 0),
                                      nullptr);
                    if (ret == 0) {
                        fmInfo() << "[AccessControlDBus::onBlockDevMounted] Successfully remounted device:" << devDesc << "with policy:" << policy << "from source:" << source;
                    } else {
                        fmCritical() << "[AccessControlDBus::onBlockDevMounted] Failed to remount device:" << devDesc << "with policy:" << policy << "errno:" << errno << "error:" << strerror(errno);
                    }
                });
            }
        }
        if (policy != kPolicyRw)   // 当格式化后，挂载点属组变为了 root 且权限变为了 755，其他用户变得无权限访问设备，因此这里在控制权限为 RW 的时候，继续执行后续变更挂载点权限的代码；
            return;
    }

    QStringList mountOpts = dev->getProperty(Property::kBlockUserspaceMountOptions).toStringList();
    fmDebug() << "[AccessControlDBus::onBlockDevMounted] Mount options for device:" << deviceId << "options:" << mountOpts;
    if (mountOpts.contains("uhelper=udisks2")) {   // only chmod for those devices mounted by udisks
        fmInfo() << "[AccessControlDBus::onBlockDevMounted] Setting full access permissions for udisks2 mounted device at:" << mountPoint;
        ::Utils::setFileMode(mountPoint, ACCESSPERMS);   // 777
    }
}

void AccessControlDBus::initConnect()
{
    DFM_MOUNT_USE_NS
    monitor.reset(new DBlockMonitor(this));
    monitor->startMonitor();
    connect(monitor.data(), &DBlockMonitor::deviceAdded, this, &AccessControlDBus::onBlockDevAdded);
    connect(monitor.data(), &DBlockMonitor::mountAdded, this, &AccessControlDBus::onBlockDevMounted);
}

void AccessControlDBus::changeMountedOnInit()
{
    // 在启动系统的时候对已挂载的设备执行一次策略变更（设备的接入先于 daemon 的启动）
    fmInfo() << "[AccessControlDBus::changeMountedOnInit] Starting access control policy application for mounted devices";
    if (globalDevPolicies.contains(kTypeBlock)) {
        fmInfo() << "[AccessControlDBus::changeMountedOnInit] Applying block device policies";
        changeMountedBlock(globalDevPolicies.value(kTypeBlock).second, "");
    }
    if (globalDevPolicies.contains(kTypeOptical)) {
        fmInfo() << "[AccessControlDBus::changeMountedOnInit] Applying optical device policies";
        changeMountedOptical(globalDevPolicies.value(kTypeOptical).second, "");
    }
    if (globalDevPolicies.contains(kTypeProtocol)) {
        fmInfo() << "[AccessControlDBus::changeMountedOnInit] Applying protocol device policies";
        changeMountedProtocol(globalDevPolicies.value(kTypeProtocol).second, "");
    }
    fmInfo() << "[AccessControlDBus::changeMountedOnInit] Completed access control policy application for mounted devices";
}

void AccessControlDBus::changeMountedBlock(int mode, const QString &device)
{
    Q_UNUSED(device)

    DFM_MOUNT_USE_NS
    QStringList blockIdGroup { monitor->getDevices() };
    QList<MountArgs> waitToHandle;
    for (const auto &id : blockIdGroup) {
        auto dev = monitor->createDeviceById(id).objectCast<DBlockDevice>();
        if (!dev)
            continue;

        bool hasFs { dev->hasFileSystem() };
        QString mnt { dev->mountPoint() };

        if (!hasFs || mnt.isEmpty())
            continue;

        bool removable { dev->removable() };
        bool optical { dev->optical() };
        if (!removable || optical)
            continue;

        // 0. 检查是否是目标设备

        // 1. 检查设备是否在白名单内

        // 2. 检查挂载点权限是否与策略一致，不一致则需要更改
        int mountedMode = ::Utils::accessMode(mnt);
        if (mountedMode == mode)
            continue;

        // 3. 需要重载或卸载
        const QString &devDesc = dev->device();   // 设备描述符
        MountArgs args;
        args.devDesc = devDesc;
        args.mountPoint = mnt;
        args.fileSystem = dev->fileSystem();
        waitToHandle.append(args);
    }

    // 4. 开启线程处理重载/卸载任务
    if (waitToHandle.count() > 0) {
        QtConcurrent::run([waitToHandle, mode]() {
            for (auto dev : waitToHandle) {
                if (mode == 0) {   // unmount
                    umount(dev.mountPoint.toLocal8Bit().data());
                } else {   // remount
                    int ret = mount(dev.devDesc.toLocal8Bit().data(),
                                    dev.mountPoint.toLocal8Bit().data(),
                                    dev.fileSystem.toLocal8Bit().data(),
                                    MS_REMOUNT | (mode == kPolicyRonly ? MS_RDONLY : 0),
                                    nullptr);
                    if (ret < 0)
                        fmDebug() << "remount " << dev.devDesc << " failed: " << errno << ": " << strerror(errno);
                }
            }
        });
    }
}

void AccessControlDBus::changeMountedOptical(int mode, const QString &device)
{
    Q_UNUSED(device)

    // 只能主动关闭，不能主动打开；光驱只负责 DISABLE / RW
    if (mode != kPolicyDisable) {
        fmInfo() << "[AccessControlDBus::changeMountedOptical] Optical device policy is not disable, no action needed";
        return;
    }

    fmInfo() << "[AccessControlDBus::changeMountedOptical] Applying disable policy to optical devices";

    DFM_MOUNT_USE_NS
    QStringList blockIdGroup { monitor->getDevices() };

    for (const QString &id : blockIdGroup) {
        auto dev = monitor->createDeviceById(id).objectCast<DBlockDevice>();
        if (!dev)
            continue;

        bool opticalDrive { dev->mediaCompatibility().join(" ").contains("optical") };
        if (!opticalDrive)
            continue;

        if (!dev->mountPoint().isEmpty()) {
            fmInfo() << "[AccessControlDBus::changeMountedOptical] Unmounting optical device:" << id;
            dev->unmountAsync({}, [id, dev](bool ok, const OperationErrorInfo &err) {
                if (!ok) {
                    fmWarning() << "[AccessControlDBus::changeMountedOptical] Failed to unmount optical device:" << id << "error:" << err.message;
                } else {
                    fmInfo() << "[AccessControlDBus::changeMountedOptical] Successfully unmounted optical device:" << id;
                    QThread::msleep(500);
                    QtConcurrent::run([dev, id]() {
                        int retry = 5;
                        while (retry-- && !dev->powerOff()) {
                            fmWarning() << "[AccessControlDBus::changeMountedOptical] Failed to power off optical device:" << id << "retries left:" << retry;
                            QThread::msleep(500);
                        }
                        if (retry < 0) {
                            fmCritical() << "[AccessControlDBus::changeMountedOptical] Failed to power off optical device after all retries:" << id;
                        } else {
                            fmInfo() << "[AccessControlDBus::changeMountedOptical] Successfully powered off optical device:" << id;
                        }
                    });
                }
            });
        }
    }
}

void AccessControlDBus::changeMountedProtocol(int mode, const QString &device)
{
    Q_UNUSED(mode)
    Q_UNUSED(device)
}

bool AccessControlDBus::checkAuthentication(const QString &id)
{
    if (!PolicyKitHelper::instance()->checkAuthorization(id, message().service())) {
        fmWarning() << "[AccessControlDBus::checkAuthentication] Authentication failed for action ID:" << id << "service:" << message().service();
        return false;
    }
    fmInfo() << "[AccessControlDBus::checkAuthentication] Authentication successful for action ID:" << id;
    return true;
}
