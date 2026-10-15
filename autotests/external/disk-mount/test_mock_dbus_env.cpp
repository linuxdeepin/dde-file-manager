// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_mock_dbus_env.cpp - 私有 DBus 总线上的 DeviceManager / Notifications mock 服务

#include "test_mock_dbus_env.h"

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>
#include <QDBusError>
#include <QDBusVariant>
#include <QProcess>
#include <QVariantMap>

#include <sys/prctl.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>

namespace ut {

void DevMngMock::reset()
{
    methods.clear();
    args.clear();
    failBlockList = false;
    failProtocolList = false;
    blockIds.clear();
    protoIds.clear();
    defaultData = { { QStringLiteral("Drive"), QString(kRootDrive) } };
    blockData.clear();
    backingData.clear();
    protoData.clear();
}

void NotifyMock::reset()
{
    notifyId = 77;
    methods.clear();
    args.clear();
}

namespace {

struct BusHolder {
    BusHolder() = default;
    BusHolder(const BusHolder &) = delete;
    BusHolder &operator=(const BusHolder &) = delete;

    QProcess daemon;
    QDBusConnection conn { QString() };   // 空名连接占位，boot 时重绑
    bool booted { false };

    ~BusHolder()
    {
        if (daemon.state() == QProcess::Running) {
            daemon.kill();
            daemon.waitForFinished(1000);
        }
    }
};

BusHolder &bus()
{
    // 有意泄漏：避免静态析构顺序问题
    static BusHolder *holder = new BusHolder();
    return *holder;
}

class DeviceManagerService : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.Daemon.DeviceManager")

public Q_SLOTS:
    void DetachBlockDevice(const QString &id)
    {
        record(QStringLiteral("DetachBlockDevice"), { id });
    }
    void DetachProtocolDevice(const QString &id)
    {
        record(QStringLiteral("DetachProtocolDevice"), { id });
    }
    void DetachAllMountedDevices()
    {
        record(QStringLiteral("DetachAllMountedDevices"), {});
    }
    void StartMonitoringUsage()
    {
        record(QStringLiteral("StartMonitoringUsage"), {});
    }
    void StopMonitoringUsage()
    {
        record(QStringLiteral("StopMonitoringUsage"), {});
    }
    void RefreshDeviceUsage()
    {
        record(QStringLiteral("RefreshDeviceUsage"), {});
    }

    QStringList GetBlockDevicesIdList(int opts)
    {
        record(QStringLiteral("GetBlockDevicesIdList"), { opts });
        if (devMngMock().failBlockList)
            sendErrorReply(QDBusError::ServiceUnknown, QStringLiteral("ut-forced-failure"));
        return devMngMock().blockIds;
    }

    QStringList GetProtocolDevicesIdList()
    {
        record(QStringLiteral("GetProtocolDevicesIdList"), {});
        if (devMngMock().failProtocolList)
            sendErrorReply(QDBusError::ServiceUnknown, QStringLiteral("ut-forced-failure"));
        return devMngMock().protoIds;
    }

    QVariantMap QueryBlockDeviceInfo(const QString &id, bool reload)
    {
        record(QStringLiteral("QueryBlockDeviceInfo"), { id, reload });
        const DevMngMock &m = devMngMock();
        if (id == m.blockId)
            return m.blockData;
        if (id == m.backingId)
            return m.backingData;
        return m.defaultData;   // 根盘探测等未知路径 → UTROOT
    }

    QVariantMap QueryProtocolDeviceInfo(const QString &id, bool reload)
    {
        record(QStringLiteral("QueryProtocolDeviceInfo"), { id, reload });
        return devMngMock().protoData;
    }

Q_SIGNALS:
    void BlockDeviceMounted(const QString &id, const QString &mountPoint);
    void BlockDeviceUnmounted(const QString &id, const QString &oldMpt);
    void BlockDeviceRemoved(const QString &id, const QString &oldMpt);
    void BlockDevicePropertyChanged(const QString &id, const QString &property, const QDBusVariant &value);
    void BlockDriveAdded();
    void BlockDriveRemoved();
    void NotifyDeviceBusy(const QString &id, const QString &action);
    void ProtocolDeviceMounted(const QString &id, const QString &mountPoint);
    void ProtocolDeviceUnmounted(const QString &id, const QString &oldMpt);
    void ProtocolDeviceRemoved(const QString &id, const QString &oldMpt);
    void SizeUsedChanged(const QString &id, qlonglong total, qlonglong free);

private:
    void record(const QString &method, const QVariantList &args)
    {
        devMngMock().methods << method;
        devMngMock().args << args;
    }
};

class NotificationsService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")

public Q_SLOTS:
    uint Notify(const QString &appName, uint replacesId, const QString &appIcon,
                const QString &summary, const QString &body, const QStringList &actions,
                const QVariantMap &hints, int expireTimeout)
    {
        notifyMock().methods << QStringLiteral("Notify");
        notifyMock().args << QVariantList { appName, replacesId, appIcon, summary, body,
                                            actions, hints, expireTimeout };
        return notifyMock().notifyId;
    }

    void CloseNotification(uint id)
    {
        notifyMock().methods << QStringLiteral("CloseNotification");
        notifyMock().args << QVariantList { id };
    }

Q_SIGNALS:
    void ActionInvoked(uint id, const QString &actionKey);
    void NotificationClosed(uint id, uint reason);
};

}   // namespace

DevMngMock &devMngMock()
{
    static DevMngMock ins;
    return ins;
}

NotifyMock &notifyMock()
{
    static NotifyMock ins;
    return ins;
}

bool ensurePrivateBus()
{
    if (bus().booted)
        return true;

    // 进程退出时必须连带杀掉 dbus-daemon：BusHolder 是有意泄漏的（见上），
    // 其析构永远不会执行；而 daemon 会继承外层 flock(run.lock) 的文件描述符，
    // 一旦成为孤儿就会把共享测试串行锁永久卡死（本批曾实际发生）。
    // 双保险：
    //   1) PR_SET_PDEATHSIG：测试进程无论正常/被杀，内核都会向 daemon 发 SIGTERM；
    //   2) std::atexit：正常退出路径兜底（BusHolder 泄漏不受影响）。
    bus().daemon.setChildProcessModifier([] {
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        // 父线程可能已在 prctl 前退出（竞态），补查一次
        if (::getppid() == 1)
            _exit(0);
    });
    static const bool exitHookInstalled = [] {
        std::atexit([] {
            if (bus().daemon.state() == QProcess::Running) {
                bus().daemon.kill();
                bus().daemon.waitForFinished(2000);
            }
        });
        return true;
    }();
    Q_UNUSED(exitHookInstalled)

    bus().daemon.start(QStringLiteral("dbus-daemon"),
                       { QStringLiteral("--session"), QStringLiteral("--print-address=1"),
                         QStringLiteral("--nofork") });
    if (!bus().daemon.waitForStarted(5000))
        return false;
    if (!bus().daemon.waitForReadyRead(5000))
        return false;
    const QString address = QString::fromLatin1(bus().daemon.readLine()).trimmed();
    if (address.isEmpty())
        return false;

    bus().conn = QDBusConnection::connectToBus(address, QStringLiteral("ut-priv-bus"));
    if (!bus().conn.isConnected())
        return false;

    static DeviceManagerService devSvc;   // 静态存储期，随总线存活
    static NotificationsService notifySvc;
    const bool okService1 = bus().conn.registerService(QStringLiteral("org.deepin.Filemanager.Daemon"));
    const bool okObj1 = bus().conn.registerObject(
            QStringLiteral("/org/deepin/Filemanager/Daemon/DeviceManager"), &devSvc,
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
    const bool okService2 = bus().conn.registerService(QStringLiteral("org.freedesktop.Notifications"));
    const bool okObj2 = bus().conn.registerObject(
            QStringLiteral("/org/freedesktop/Notifications"), &notifySvc,
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);

    bus().booted = okService1 && okObj1 && okService2 && okObj2;
    return bus().booted;
}

}   // namespace ut

#include "test_mock_dbus_env.moc"
