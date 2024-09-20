// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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

#include <dfm-mount/dblockmonitor.h>
#include <dfm-mount/dblockdevice.h>
#include <dfm-mount/base/dmountutils.h>

#include <sys/mount.h>
#include <sys/stat.h>

namespace service_accesscontrol {
DFM_LOG_REISGER_CATEGORY(SERVICEACCESSCONTROL_NAMESPACE)
}

DFMBASE_USE_NAMESPACE
SERVICEACCESSCONTROL_USE_NAMESPACE

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

void AccessControlDBus::ChangeDiskPassword(const QString &oldPwd, const QString &newPwd)
{
    if (!checkAuthentication(kPolicyKitDiskPwdActionId)) {
        fmDebug() << "Check authentication failed";
        emit DiskPasswordChecked(kAuthenticationFailed);
        return;
    }

    const auto &devList = DeviceUtils::encryptedDisks();
    if (devList.isEmpty()) {
        emit DiskPasswordChecked(kNoError);
        QTimer::singleShot(500, [this] { emit DiskPasswordChanged(kAccessDiskFailed); });
        return;
    }

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
            emit DiskPasswordChecked(kPasswordWrong);
            return;
        } else if (ret == kPasswordWrong) {
            ret = kPasswordInconsistent;
            break;
        } else if (ret == kNoError) {
            if (i == 0)
                emit DiskPasswordChecked(kNoError);

            ret = Utils::changeDiskPassword(cd, tmpOldPwd.data(), tmpNewPwd.data());
        } else {
            break;
        }

        if (ret != kNoError)
            break;

        successList << devList[i];
    }

    // restore password
    if (ret != kNoError && !successList.isEmpty()) {
        for (const auto &device : successList) {
            struct crypt_device *cd = nullptr;
            Utils::checkDiskPassword(&cd, tmpNewPwd.data(), device.toLocal8Bit().data());
            Utils::changeDiskPassword(cd, tmpNewPwd.data(), tmpOldPwd.data());
        }
    }

    emit DiskPasswordChanged(ret);
}

bool AccessControlDBus::Chmod(const QString &path, uint mode)
{
    if (!checkAuthentication(kPolicyKitChmodActionId)) {
        fmWarning() << "authenticate failed to change permission of" << path;
        return false;
    }

    if (path.isEmpty())
        return false;

    QFile f(path);
    if (!f.exists()) {
        fmWarning() << "file not exists" << path;
        return false;
    }

    fmInfo() << "start changing the access permission of" << path << mode;
    int ret = ::Utils::setFileMode(path.toStdString().c_str(), mode);
    if (ret != 0) {
        fmWarning() << "chmod for" << path << "failed due to" << strerror(errno);
        return false;
    }
    fmInfo() << "access permission for" << path << "is modified successfully";
    return true;
}

void AccessControlDBus::onBlockDevAdded(const QString &deviceId)
{
    DFM_MOUNT_USE_NS
    auto dev = monitor->createDeviceById(deviceId).objectCast<DBlockDevice>();
    if (!dev) {
        fmWarning() << "cannot craete device handler for " << deviceId;
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
        QtConcurrent::run([deviceId, dev]() {
            int retry = 5;
            while (retry-- && !dev->powerOff()) {
                fmWarning() << "poweroff device failed: " << deviceId << dev->lastError().message;
                QThread::msleep(500);
            }
        });
    }
}

void AccessControlDBus::onBlockDevMounted(const QString &deviceId, const QString &mountPoint)
{
    DFM_MOUNT_USE_NS
    auto dev = monitor->createDeviceById(deviceId).objectCast<DBlockDevice>();
    if (!dev || dev->hintSystem()) {
        fmWarning() << "cannot create device or device is system disk" << deviceId;
        return;
    }

    if (globalDevPolicies.contains(kTypeBlock)) {
        QString devDesc { dev->device() };
        int mode = ::Utils::accessMode(mountPoint);
        QString source = globalDevPolicies.value(kTypeBlock).first;
        int policy = globalDevPolicies.value(kTypeBlock).second;
        QString fs { dev->fileSystem() };
        if (mode != policy) {
            if (policy == kPolicyDisable) {
                // unmount
            } else {
                // remount
                QtConcurrent::run([devDesc, mountPoint, fs, policy, source]() {
                    int ret = ::mount(devDesc.toLocal8Bit().data(),
                                      mountPoint.toLocal8Bit().data(),
                                      fs.toLocal8Bit().data(),
                                      MS_REMOUNT | (policy == kPolicyRonly ? MS_RDONLY : 0),
                                      nullptr);
                    if (ret == 0)
                        fmDebug() << "remount with policy " << policy << " from " << source;
                    else
                        fmDebug() << "remount with policy " << policy << " failed, errno: " << errno << ", errstr: " << strerror(errno);
                });
            }
        }
        if (policy != kPolicyRw)   // 当格式化后，挂载点属组变为了 root 且权限变为了 755，其他用户变得无权限访问设备，因此这里在控制权限为 RW 的时候，继续执行后续变更挂载点权限的代码；
            return;
    }

    QStringList mountOpts = dev->getProperty(Property::kBlockUserspaceMountOptions).toStringList();
    fmDebug() << "mount opts: ==>" << mountOpts << deviceId;
    if (mountOpts.contains("uhelper=udisks2"))   // only chmod for those devices mounted by udisks
        ::Utils::setFileMode(mountPoint, ACCESSPERMS);   // 777
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
    fmDebug() << "start change access on init...";
    if (globalDevPolicies.contains(kTypeBlock))
        changeMountedBlock(globalDevPolicies.value(kTypeBlock).second, "");
    if (globalDevPolicies.contains(kTypeOptical))
        changeMountedOptical(globalDevPolicies.value(kTypeOptical).second, "");
    if (globalDevPolicies.contains(kTypeProtocol))
        changeMountedProtocol(globalDevPolicies.value(kTypeProtocol).second, "");
    fmDebug() << "end change access on init...";
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
    if (mode != kPolicyDisable)
        return;

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
            dev->unmountAsync({}, [id, dev](bool ok, const OperationErrorInfo &err) {
                if (!ok) {
                    fmDebug() << "Error occured while unmount optical device: " << id << err.message;
                } else {
                    QThread::msleep(500);
                    QtConcurrent::run([dev, id]() {
                        int retry = 5;
                        while (retry-- && !dev->powerOff()) {
                            fmDebug() << "Error occured while poweroff optical device: " << id;
                            QThread::msleep(500);
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
        fmInfo() << "Authentication failed !!";
        return false;
    }
    return true;
}
