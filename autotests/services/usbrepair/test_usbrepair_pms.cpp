// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// usbrepair 服务单元测试：服务以共享库/插件形态构建且无独立可链测试桩目标，
// 此处 unity-include 被测服务实现源码（main.cpp 除外），并对 DBus / QProcess /
// polkit 等外部依赖打桩隔离。

#include <gtest/gtest.h>
#include <stubext.h>

#include <dlfcn.h>

#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusInterface>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>

#include <polkit-qt6-1/PolkitQt1/Authority>

#include "services/usbrepair/private/usbrepairdbus_p.h"
#include "services/usbrepair/usbrepairdbus.h"
#include "services/usbrepair/usbrepairmonitor.h"
#include "services/usbrepair/usbrepairworker.h"

// unity-include 被测实现（服务无可链接目标）
#include "services/usbrepair/plugin.cpp"
#include "services/usbrepair/usbrepairdbus.cpp"
#include "services/usbrepair/usbrepairmonitor.cpp"
#include "services/usbrepair/usbrepairworker.cpp"

SERVICEUSBREPAIR_USE_NAMESPACE

namespace {
// ------------------------- QProcess 桩的全局状态 -------------------------
struct UtProcCtl
{
    QVector<QByteArray> outs;   // 每次 start() 依次返回的标准输出
    QVector<QByteArray> errs;   // 每次 start() 依次返回的标准错误
    QVector<int> exits;   // 每次 start() 依次返回的退出码
    QMap<quint64, int> idxOf;   // QProcess 对象 -> 序号
    int next { 0 };
    bool waitFinished { true };
    bool waitStarted { true };
    int killCalls { 0 };
    int startCalls { 0 };

    int idx(void *proc) const
    {
        const int i = idxOf.value(reinterpret_cast<quint64>(proc), qMax(next - 1, 0));
        return qMin(i, qMax(outs.size() - 1, 0));
    }
    void reset()
    {
        outs.clear();
        errs.clear();
        exits.clear();
        idxOf.clear();
        next = 0;
        waitFinished = true;
        waitStarted = true;
        killCalls = 0;
        startCalls = 0;
    }
};
UtProcCtl g_proc;

// ------------------------- DBus 属性桩的全局状态 -------------------------
QMap<QByteArray, QVariant> g_props;
bool g_ifaceValid { false };
int g_propertyCalls { 0 };

// ------------------------- 各测试计数器 -------------------------
int g_healthCalls { 0 };
int g_missingFsCalls { 0 };
int g_authYes { 0 };   // polkit 授权结果: 0 = No, 1 = Yes
int g_killCallsWorker { 0 };

void resetGlobals()
{
    g_proc.reset();
    g_props.clear();
    g_ifaceValid = false;
    g_propertyCalls = 0;
    g_healthCalls = 0;
    g_missingFsCalls = 0;
    g_authYes = 0;
    g_killCallsWorker = 0;
}

// 等待 ms 毫秒并持续派发事件（驱动 QTimer::singleShot 的 lambda 执行）
void pumpEvents(int ms)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(20);
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
}
}   // namespace


namespace {
// QFile::open 为跨库虚函数：成员指针只是 vtable 偏移，无法直接取地址打桩，
// 通过 dlsym 解析其导出符号地址再 patch（虚分派与直接调用都会命中）。
template <typename FnPtr, typename Lamda>
bool stubDlsym(stub_ext::StubExt &stub, const char *mangled, Lamda fake)
{
    void *addr = ::dlsym(RTLD_DEFAULT, mangled);
    return addr ? stub.set_lamda(reinterpret_cast<FnPtr>(addr), fake) : false;
}
}   // namespace

class UsbRepairServiceTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        resetGlobals();

        // QDBusContext::message() 在无 DBus 上下文时会解引用空指针，桩为空消息
        static QDBusMessage s_emptyDBusMsg;
        stub.set_lamda(static_cast<const QDBusMessage &(QDBusContext::*)() const>(&QDBusContext::message),
                       [](QDBusContext *) -> const QDBusMessage & {
            return s_emptyDBusMsg;
        });


        // 隔离真实 DBus：systemBus / connectToBus 一律返回未连接的连接对象
        stub.set_lamda(static_cast<QDBusConnection (*)()>(&QDBusConnection::systemBus),
                       []() -> QDBusConnection {
            return QDBusConnection(QString());
        });
        stub.set_lamda(
                static_cast<QDBusConnection (*)(QDBusConnection::BusType, const QString &)>(&QDBusConnection::connectToBus),
                [](QDBusConnection::BusType, const QString &) -> QDBusConnection {
            return QDBusConnection(QString());
        });

        // QDBusInterface 属性访问桩（按属性名分发）
        stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                       [](QDBusAbstractInterface *) -> bool {
            return g_ifaceValid;
        });
        stub.set_lamda(static_cast<QVariant (QDBusAbstractInterface::*)(const char *) const>(&QDBusAbstractInterface::property),
                       [](QDBusAbstractInterface *, const char *name) -> QVariant {
            ++g_propertyCalls;
            return g_props.value(name);
        });

        // QProcess 桩：不启动真实外部进程
        stub.set_lamda(
                static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODeviceBase::OpenMode)>(&QProcess::start),
                [](QProcess *self, const QString &, const QStringList &, QIODeviceBase::OpenMode) {
            ++g_proc.startCalls;
            g_proc.idxOf[reinterpret_cast<quint64>(self)] = g_proc.next++;
        });
        stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished),
                       [](QProcess *, int) -> bool { return g_proc.waitFinished; });
        stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForStarted),
                       [](QProcess *, int) -> bool { return g_proc.waitStarted; });
        stub.set_lamda(static_cast<int (QProcess::*)() const>(&QProcess::exitCode),
                       [](QProcess *self) -> int {
            const int i = g_proc.idx(self);
            return g_proc.exits.value(i, 0);
        });
        stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                       [](QProcess *self) -> QByteArray {
            const int i = g_proc.idx(self);
            return g_proc.outs.value(i, QByteArray());
        });
        stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardError),
                       [](QProcess *self) -> QByteArray {
            const int i = g_proc.idx(self);
            return g_proc.errs.value(i, QByteArray());
        });
        stub.set_lamda(static_cast<void (QProcess::*)()>(&QProcess::kill),
                       [](QProcess *) { ++g_proc.killCalls; });
    }

    void TearDown() override
    {
        stub.clear();
        resetGlobals();
    }

    // 配置 polkit 授权桩（usbrepairworker.cpp 直接调用 PolkitQt1::Authority）
    void setupPolkit(bool yes)
    {
        g_authYes = yes ? 1 : 0;
        stub.set_lamda(static_cast<PolkitQt1::Authority *(*)(PolkitAuthority *)>(&PolkitQt1::Authority::instance),
                       [](PolkitAuthority *) -> PolkitQt1::Authority * {
            return reinterpret_cast<PolkitQt1::Authority *>(quintptr(0x1));
        });
        stub.set_lamda(&PolkitQt1::Authority::checkAuthorizationSync,
                       [](PolkitQt1::Authority *, const QString &, const PolkitQt1::Subject &,
                          PolkitQt1::Authority::AuthorizationFlags) -> PolkitQt1::Authority::Result {
            return g_authYes ? PolkitQt1::Authority::Yes : PolkitQt1::Authority::No;
        });
    }
};

/******************************************************************************/
/**************************** 全局与插件入口 **********************************/
/******************************************************************************/

TEST_F(UsbRepairServiceTest, LogCategory_RegisteredAndFetchable)
{
    // Arrange
    const QLoggingCategory *viaHelper = &service_usbrepair::__getLogCategoryName();

    // Act
    const QLoggingCategory *registered = &service_usbrepair::__logservice_usbrepair();

    // Assert
    EXPECT_EQ(viaHelper, registered);
    EXPECT_STREQ(registered->categoryName(), "org.deepin.dde.filemanager.plugin.service_usbrepair");
}

TEST_F(UsbRepairServiceTest, DsmRegister_CreatesServiceAndUnRegisterCleansUp)
{
    // Arrange
    EXPECT_EQ(DSMRegister("ut.usbrepair.service", nullptr), 0);

    // Act
    const int unreg = DSMUnRegister("ut.usbrepair.service", nullptr);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    // Assert
    EXPECT_EQ(unreg, 0);
    EXPECT_EQ(DSMRegister("ut.usbrepair.service", nullptr), 0);
    EXPECT_EQ(DSMUnRegister("ut.usbrepair.service", nullptr), 0);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_F(UsbRepairServiceTest, TR_StaticTranslations_FallBackToSource)
{
    // Arrange / Act：未加载翻译 catalog 时 tr 应回显源文
    const QString monitorTr = service_usbrepair::UsbRepairMonitor::tr("Repair");
    const QString workerTr = service_usbrepair::UsbRepairWorker::tr("Repair");

    // Assert
    EXPECT_EQ(monitorTr, QStringLiteral("Repair"));
    EXPECT_EQ(workerTr, QStringLiteral("Repair"));
}

/******************************************************************************/
/**************************** UsbRepairDBus ***********************************/
/******************************************************************************/

TEST_F(UsbRepairServiceTest, UsbRepairDBus_Lifecycle_AndInitConnectForwardsSignals)
{
    // Arrange
    UsbRepairDBus dbus("ut.usbrepair.dbus");
    QSignalSpy fsErrSpy(&dbus, &UsbRepairDBus::FsErrorDetected);
    QSignalSpy fsClrSpy(&dbus, &UsbRepairDBus::FsErrorCleared);
    QSignalSpy progSpy(&dbus, &UsbRepairDBus::RepairProgress);
    QSignalSpy finSpy(&dbus, &UsbRepairDBus::RepairFinished);

    // Act
    emit dbus.d->monitor->errorDetected("/dev/sdz1", "UT Disk", "vfat", "dirty_bit", true, "msg");
    emit dbus.d->monitor->errorCleared("/dev/sdz1");
    emit dbus.d->worker->progress("/dev/sdz1", 42, "line");
    emit dbus.d->worker->finished("/dev/sdz1", true, "ok");
    dbus.cleanup();

    // Assert
    EXPECT_EQ(fsErrSpy.count(), 1);
    EXPECT_EQ(fsClrSpy.count(), 1);
    EXPECT_EQ(progSpy.count(), 1);
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_EQ(progSpy.takeFirst().at(1).toInt(), 42);
    EXPECT_TRUE(finSpy.takeFirst().at(1).toBool());
    // cleanup 后监控对象已停止（ObjMgr 清空）
    EXPECT_EQ(dbus.d->monitor->m_udisks2ObjMgr, nullptr);
}

TEST_F(UsbRepairServiceTest, Repair_ForwardsToWorkerStartRepair)
{
    // Arrange
    static QString gotDevice;
    static QString gotCaller;
    gotDevice.clear();
    gotCaller.clear();
    stub.set_lamda(&UsbRepairWorker::startRepair,
                   [](UsbRepairWorker *, const QString &dev, const QString &caller, QString &err) -> bool {
        gotDevice = dev;
        gotCaller = caller;
        err = "stub-ok";
        return true;
    });
    UsbRepairDBus dbus("ut.usbrepair.dbus");

    // Act
    QString errMsg;
    const bool ok = dbus.Repair("/dev/sdz1", errMsg);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(gotDevice, "/dev/sdz1");
    EXPECT_EQ(errMsg, "stub-ok");
    EXPECT_TRUE(gotCaller.isEmpty());   // 非 DBus 上下文调用时 caller 为空
}

TEST_F(UsbRepairServiceTest, CancelRepair_ForwardsToWorkerCancelRepair)
{
    // Arrange
    static int cancelCalls = 0;
    cancelCalls = 0;
    stub.set_lamda(&UsbRepairWorker::cancelRepair,
                   [](UsbRepairWorker *, const QString &, const QString &) -> bool {
        ++cancelCalls;
        return false;
    });
    UsbRepairDBus dbus("ut.usbrepair.dbus");

    // Act
    const bool ok = dbus.CancelRepair("/dev/sdz1");

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(cancelCalls, 1);
}

/******************************************************************************/
/***************************** UsbRepairMonitor *******************************/
/******************************************************************************/

TEST_F(UsbRepairServiceTest, Monitor_Lifecycle_StartStopIdempotent)
{
    // Arrange
    UsbRepairMonitor monitor;

    // Act
    monitor.startMonitoring();
    monitor.startMonitoring();   // 二次启动应幂等
    monitor.stopMonitoring();
    monitor.stopMonitoring();   // 二次停止应幂等

    // Assert
    EXPECT_EQ(monitor.m_udisks2ObjMgr, nullptr);
}

TEST_F(UsbRepairServiceTest, OnInterfacesAdded_FilesystemIface_SchedulesHealthCheck)
{
    // Arrange
    UsbRepairMonitor monitor;
    stub.set_lamda(&UsbRepairMonitor::checkDeviceHealth,
                   [](UsbRepairMonitor *, const QString &) { ++g_healthCalls; });
    QMap<QString, QVariantMap> ifaces;
    ifaces.insert(Defines::kUdisks2FilesystemIface, {});

    // Act
    monitor.onInterfacesAdded(QDBusObjectPath("/org/freedesktop/UDisks2/block_devices/sdz1"), ifaces);
    pumpEvents(Defines::kMountSettleDelayMs + 600);

    // Assert
    EXPECT_EQ(g_healthCalls, 1);
    EXPECT_EQ(g_missingFsCalls, 0);
}

TEST_F(UsbRepairServiceTest, OnInterfacesAdded_BlockOnlyIface_SchedulesMissingFsCheck)
{
    // Arrange
    UsbRepairMonitor monitor;
    stub.set_lamda(&UsbRepairMonitor::checkDeviceHealth,
                   [](UsbRepairMonitor *, const QString &) { ++g_healthCalls; });
    stub.set_lamda(&UsbRepairMonitor::checkMissingFilesystem,
                   [](UsbRepairMonitor *, const QString &) { ++g_missingFsCalls; });
    QMap<QString, QVariantMap> ifaces;
    ifaces.insert(Defines::kUdisks2BlockIface, {});

    // Act
    monitor.onInterfacesAdded(QDBusObjectPath("/org/freedesktop/UDisks2/block_devices/sdz2"), ifaces);
    pumpEvents(Defines::kMountSettleDelayMs + 600);

    // Assert
    EXPECT_EQ(g_missingFsCalls, 1);
    EXPECT_EQ(g_healthCalls, 0);
}

TEST_F(UsbRepairServiceTest, OnInterfacesAdded_UnknownIface_DoesNothing)
{
    // Arrange
    UsbRepairMonitor monitor;
    stub.set_lamda(&UsbRepairMonitor::checkDeviceHealth,
                   [](UsbRepairMonitor *, const QString &) { ++g_healthCalls; });
    stub.set_lamda(&UsbRepairMonitor::checkMissingFilesystem,
                   [](UsbRepairMonitor *, const QString &) { ++g_missingFsCalls; });
    QMap<QString, QVariantMap> ifaces;
    ifaces.insert("org.freedesktop.DBus.Properties", {});

    // Act
    monitor.onInterfacesAdded(QDBusObjectPath("/org/freedesktop/UDisks2/block_devices/sdz3"), ifaces);
    pumpEvents(200);

    // Assert
    EXPECT_EQ(g_healthCalls, 0);
    EXPECT_EQ(g_missingFsCalls, 0);
}

TEST_F(UsbRepairServiceTest, OnInterfacesRemoved_EmitsErrorCleared)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorCleared);

    // Act
    monitor.onInterfacesRemoved(QDBusObjectPath("/org/freedesktop/UDisks2/block_devices/sdz1"),
                                { Defines::kUdisks2FilesystemIface });

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), "/org/freedesktop/UDisks2/block_devices/sdz1");
}

TEST_F(UsbRepairServiceTest, CheckDeviceHealth_IgnoredDevice_NoSignal)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return true; });

    // Act
    monitor.checkDeviceHealth("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(g_propertyCalls, 0);
}

TEST_F(UsbRepairServiceTest, CheckDeviceHealth_NonUsbDevice_NoSignal)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *) -> bool { return false; });

    // Act
    monitor.checkDeviceHealth("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UsbRepairServiceTest, CheckDeviceHealth_EmptyDeviceFile_NoSignal)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *) -> bool { return true; });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return ""; });

    // Act
    monitor.checkDeviceHealth("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UsbRepairServiceTest, CheckDeviceHealth_WholeDiskDevice_NoSignal)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *) -> bool { return true; });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "/dev/sdz"; });

    // Act
    monitor.checkDeviceHealth("/org/freedesktop/UDisks2/block_devices/sdz");

    // Assert
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UsbRepairServiceTest, CheckDeviceHealth_UndetectableFs_EmitsUnknownError)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *name) -> bool {
        if (name)
            *name = "UT Disk";
        return true;
    });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "/dev/sdz1"; });
    stub.set_lamda(&UsbRepairMonitor::getFsType,
                   [](UsbRepairMonitor *, const QString &) -> QString { return ""; });

    // Act
    monitor.checkDeviceHealth("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), "/dev/sdz1");
    EXPECT_EQ(args.at(1).toString(), "UT Disk");
    EXPECT_EQ(args.at(2).toString(), "unknown");
    EXPECT_EQ(args.at(3).toString(), Defines::kErrorTypeMountFailed);
    EXPECT_FALSE(args.at(4).toBool());
}

TEST_F(UsbRepairServiceTest, CheckDeviceHealth_UnsupportedFs_NoSignal)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *) -> bool { return true; });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "/dev/sdz1"; });
    stub.set_lamda(&UsbRepairMonitor::getFsType,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "btrfs"; });

    // Act
    monitor.checkDeviceHealth("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UsbRepairServiceTest, CheckDeviceHealth_HardwareReadOnly_EmitsReadOnlyError)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *name) -> bool {
        if (name)
            *name = "UT Disk";
        return true;
    });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "/dev/sdz1"; });
    stub.set_lamda(&UsbRepairMonitor::getFsType,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "vfat"; });
    stub.set_lamda(&UsbRepairMonitor::isHardwareReadOnly,
                   [](UsbRepairMonitor *, const QString &) -> bool { return true; });

    // Act
    monitor.checkDeviceHealth("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_EQ(args.at(2).toString(), "vfat");
    EXPECT_EQ(args.at(3).toString(), Defines::kErrorTypeReadOnly);
    EXPECT_FALSE(args.at(4).toBool());
}

TEST_F(UsbRepairServiceTest, CheckDeviceHealth_MountedHealthy_NoSignal)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *) -> bool { return true; });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "/dev/sdz1"; });
    stub.set_lamda(&UsbRepairMonitor::getFsType,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "vfat"; });
    stub.set_lamda(&UsbRepairMonitor::isHardwareReadOnly,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isDeviceMounted,
                   [](UsbRepairMonitor *, const QString &) -> bool { return true; });

    // Act
    monitor.checkDeviceHealth("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UsbRepairServiceTest, CheckDeviceHealth_DirtyBit_EmitsRepairableError)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *name) -> bool {
        if (name)
            *name = "UT Disk";
        return true;
    });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "/dev/sdz1"; });
    stub.set_lamda(&UsbRepairMonitor::getFsType,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "vfat"; });
    stub.set_lamda(&UsbRepairMonitor::isHardwareReadOnly,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isDeviceMounted,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::checkDirtyBit,
                   [](UsbRepairMonitor *, const QString &, const QString &) -> bool { return true; });

    // Act
    monitor.checkDeviceHealth("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_EQ(args.at(3).toString(), Defines::kErrorTypeMountFailed);
    EXPECT_TRUE(args.at(4).toBool());   // can repair
    EXPECT_FALSE(args.at(5).toString().isEmpty());
}

TEST_F(UsbRepairServiceTest, IsUsbDevice_InvalidInterface_ReturnsFalse)
{
    // Arrange
    UsbRepairMonitor monitor;   // g_ifaceValid = false
    QString name;

    // Act
    const bool isUsb = monitor.isUsbDevice("/org/freedesktop/UDisks2/block_devices/sdz1", &name);

    // Assert
    EXPECT_FALSE(isUsb);
    EXPECT_TRUE(name.isEmpty());
}

TEST_F(UsbRepairServiceTest, IsUsbDevice_NoAssociatedDrive_ReturnsFalse)
{
    // Arrange
    g_ifaceValid = true;
    g_props.insert("Drive", QVariant::fromValue(QDBusObjectPath("/")));
    UsbRepairMonitor monitor;

    // Act
    const bool isUsb = monitor.isUsbDevice("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_FALSE(isUsb);
}

TEST_F(UsbRepairServiceTest, IsUsbDevice_NonUsbBus_ReturnsFalse)
{
    // Arrange
    g_ifaceValid = true;
    g_props.insert("Drive", QVariant::fromValue(QDBusObjectPath("/org/freedesktop/UDisks2/drives/UTDrive")));
    g_props.insert("ConnectionBus", "sata");
    UsbRepairMonitor monitor;

    // Act
    const bool isUsb = monitor.isUsbDevice("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_FALSE(isUsb);
}

TEST_F(UsbRepairServiceTest, IsUsbDevice_OpticalDrive_ReturnsFalse)
{
    // Arrange
    g_ifaceValid = true;
    g_props.insert("Drive", QVariant::fromValue(QDBusObjectPath("/org/freedesktop/UDisks2/drives/UTDrive")));
    g_props.insert("ConnectionBus", "usb");
    g_props.insert("Optical", true);
    UsbRepairMonitor monitor;

    // Act
    const bool isUsb = monitor.isUsbDevice("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_FALSE(isUsb);
}

TEST_F(UsbRepairServiceTest, IsUsbDevice_UsbNotRemovable_ReturnsFalse)
{
    // Arrange
    g_ifaceValid = true;
    g_props.insert("Drive", QVariant::fromValue(QDBusObjectPath("/org/freedesktop/UDisks2/drives/UTDrive")));
    g_props.insert("ConnectionBus", "usb");
    g_props.insert("Optical", false);
    g_props.insert("Removable", false);
    UsbRepairMonitor monitor;

    // Act
    const bool isUsb = monitor.isUsbDevice("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_FALSE(isUsb);
}

TEST_F(UsbRepairServiceTest, IsUsbDevice_RemovableUsb_ReturnsTrueWithDeviceName)
{
    // Arrange
    g_ifaceValid = true;
    g_props.insert("Drive", QVariant::fromValue(QDBusObjectPath("/org/freedesktop/UDisks2/drives/UTDrive")));
    g_props.insert("ConnectionBus", "usb");
    g_props.insert("Optical", false);
    g_props.insert("Removable", true);
    g_props.insert("Id", "fallback-id");
    g_props.insert("Vendor", "UTVendor");
    g_props.insert("Model", "UTModel");
    UsbRepairMonitor monitor;
    QString name;

    // Act
    const bool isUsb = monitor.isUsbDevice("/org/freedesktop/UDisks2/block_devices/sdz1", &name);

    // Assert
    EXPECT_TRUE(isUsb);
    EXPECT_EQ(name, "UTVendor UTModel");
}

TEST_F(UsbRepairServiceTest, GetDeviceFile_InvalidInterface_ReturnsEmpty)
{
    // Arrange
    UsbRepairMonitor monitor;   // g_ifaceValid = false

    // Act
    const QString dev = monitor.getDeviceFile("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_TRUE(dev.isEmpty());
    EXPECT_EQ(g_propertyCalls, 0);
}

TEST_F(UsbRepairServiceTest, GetDeviceFile_TrimsTrailingNul)
{
    // Arrange
    g_ifaceValid = true;
    QString withNul("/dev/sdz1");
    withNul.append(QChar::Null);
    g_props.insert("Device", withNul);
    UsbRepairMonitor monitor;

    // Act
    const QString dev = monitor.getDeviceFile("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_EQ(dev, "/dev/sdz1");
    EXPECT_FALSE(dev.endsWith(QChar::Null));
}

TEST_F(UsbRepairServiceTest, GetFsType_NormalizesExt23ToExt4)
{
    // Arrange
    g_proc.outs = { QByteArray("ext2") };
    g_proc.exits = { 0 };
    UsbRepairMonitor monitor;

    // Act
    const QString fs = monitor.getFsType("/dev/sdz1");

    // Assert
    EXPECT_EQ(fs, "ext4");
}

TEST_F(UsbRepairServiceTest, GetFsType_PassesPlainTypeThrough)
{
    // Arrange
    g_proc.outs = { QByteArray("vfat") };
    g_proc.exits = { 0 };
    UsbRepairMonitor monitor;

    // Act
    const QString fs = monitor.getFsType("/dev/sdz1");

    // Assert
    EXPECT_EQ(fs, "vfat");
}

TEST_F(UsbRepairServiceTest, GetFsType_BlkidFails_ReturnsEmpty)
{
    // Arrange
    g_proc.outs = { QByteArray("") };
    g_proc.exits = { 1 };
    UsbRepairMonitor monitor;

    // Act
    const QString fs = monitor.getFsType("/dev/sdz1");

    // Assert
    EXPECT_TRUE(fs.isEmpty());
}

TEST_F(UsbRepairServiceTest, IsDeviceMounted_NotListed_ReturnsFalse)
{
    // Arrange
    UsbRepairMonitor monitor;

    // Act
    const bool mounted = monitor.isDeviceMounted("/dev/ut-not-mounted-9999");

    // Assert
    EXPECT_FALSE(mounted);
}

TEST_F(UsbRepairServiceTest, IsDeviceMounted_ListedInMounts_ReturnsTrue)
{
    // Arrange
    static const char *kFakeMounts = "/dev/sdz9 /media/ut/UDISK vfat rw,relatime 0 0\n";
    stubDlsym<bool (*)(QFile *, QIODevice::OpenMode)>(
                   stub, "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE",
                   [](QFile *, QIODevice::OpenMode) -> bool { return true; });
    stub.set_lamda(static_cast<QByteArray (QIODevice::*)()>(&QIODevice::readAll),
                   [](QIODevice *) -> QByteArray { return kFakeMounts; });
    UsbRepairMonitor monitor;

    // Act
    const bool mounted = monitor.isDeviceMounted("/dev/sdz9");
    const bool other = monitor.isDeviceMounted("/dev/sdz8");

    // Assert
    EXPECT_TRUE(mounted);
    EXPECT_FALSE(other);
}

TEST_F(UsbRepairServiceTest, IsDeviceMounted_OpenFails_ReturnsFalse)
{
    // Arrange
    stubDlsym<bool (*)(QFile *, QIODevice::OpenMode)>(
                   stub, "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE",
                   [](QFile *, QIODevice::OpenMode) -> bool { return false; });
    UsbRepairMonitor monitor;

    // Act
    const bool mounted = monitor.isDeviceMounted("/dev/sdz9");

    // Assert
    EXPECT_FALSE(mounted);
}

TEST_F(UsbRepairServiceTest, IsHardwareReadOnly_RoOutput_ReturnsTrue)
{
    // Arrange
    g_proc.outs = { QByteArray("1") };
    g_proc.exits = { 0 };
    UsbRepairMonitor monitor;

    // Act
    const bool ro = monitor.isHardwareReadOnly("/dev/sdz9");

    // Assert
    EXPECT_TRUE(ro);
    EXPECT_EQ(g_proc.startCalls, 1);
}

TEST_F(UsbRepairServiceTest, IsHardwareReadOnly_RwOutput_ReturnsFalse)
{
    // Arrange
    g_proc.outs = { QByteArray("0") };
    g_proc.exits = { 0 };
    UsbRepairMonitor monitor;

    // Act
    const bool ro = monitor.isHardwareReadOnly("/dev/sdz9");

    // Assert
    EXPECT_FALSE(ro);
}

TEST_F(UsbRepairServiceTest, IsHardwareReadOnly_CommandFails_ReturnsFalse)
{
    // Arrange
    g_proc.outs = { QByteArray("") };
    g_proc.exits = { 1 };
    UsbRepairMonitor monitor;

    // Act
    const bool ro = monitor.isHardwareReadOnly("/dev/sdz9");

    // Assert
    EXPECT_FALSE(ro);
}

TEST_F(UsbRepairServiceTest, CheckDirtyBit_ExitPositive_ReturnsTrue)
{
    // Arrange
    g_proc.exits = { 1 };
    UsbRepairMonitor monitor;

    // Act
    const bool dirty = monitor.checkDirtyBit("/dev/sdz1", "vfat");

    // Assert
    EXPECT_TRUE(dirty);
    EXPECT_EQ(g_proc.startCalls, 1);
}

TEST_F(UsbRepairServiceTest, CheckDirtyBit_ExitZero_ReturnsFalse)
{
    // Arrange
    g_proc.exits = { 0 };
    UsbRepairMonitor monitor;

    // Act
    const bool dirty = monitor.checkDirtyBit("/dev/sdz1", "exfat");

    // Assert
    EXPECT_FALSE(dirty);
}

TEST_F(UsbRepairServiceTest, CheckDirtyBit_CrashedTool_ReturnsFalse)
{
    // Arrange
    g_proc.exits = { -1 };
    UsbRepairMonitor monitor;

    // Act
    const bool dirty = monitor.checkDirtyBit("/dev/sdz1", "ntfs");

    // Assert
    EXPECT_FALSE(dirty);
}

TEST_F(UsbRepairServiceTest, CheckDirtyBit_UnsupportedFs_SkipsTool)
{
    // Arrange
    UsbRepairMonitor monitor;

    // Act
    const bool dirty = monitor.checkDirtyBit("/dev/sdz1", "zfs");

    // Assert
    EXPECT_FALSE(dirty);
    EXPECT_EQ(g_proc.startCalls, 0);
}

TEST_F(UsbRepairServiceTest, CheckMissingFilesystem_IgnoredDevice_NoSignal)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return true; });

    // Act
    monitor.checkMissingFilesystem("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(g_proc.startCalls, 0);
}

TEST_F(UsbRepairServiceTest, CheckMissingFilesystem_NonUsbDevice_NoSignal)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *) -> bool { return false; });

    // Act
    monitor.checkMissingFilesystem("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(g_proc.startCalls, 0);
}

TEST_F(UsbRepairServiceTest, CheckMissingFilesystem_NoDeviceFile_Skips)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *) -> bool { return true; });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return ""; });

    // Act
    monitor.checkMissingFilesystem("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(g_proc.startCalls, 0);
}

TEST_F(UsbRepairServiceTest, CheckMissingFilesystem_WholeDisk_Skips)
{
    // Arrange
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *) -> bool { return true; });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "/dev/sdz"; });

    // Act
    monitor.checkMissingFilesystem("/org/freedesktop/UDisks2/block_devices/sdz");

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(g_proc.startCalls, 0);
}

TEST_F(UsbRepairServiceTest, CheckMissingFilesystem_FsAppearedLater_DelegatesToHealthCheck)
{
    // Arrange
    g_ifaceValid = true;
    g_props.insert("MountPoints", QVariant::fromValue(QList<QByteArray> { "/media/ut/disk" }));
    UsbRepairMonitor monitor;
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *) -> bool { return true; });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "/dev/sdz1"; });
    stub.set_lamda(&UsbRepairMonitor::checkDeviceHealth,
                   [](UsbRepairMonitor *, const QString &) { ++g_healthCalls; });

    // Act
    monitor.checkMissingFilesystem("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_EQ(g_healthCalls, 1);
    EXPECT_EQ(g_proc.startCalls, 0);
}

TEST_F(UsbRepairServiceTest, CheckMissingFilesystem_FileCmdDetectsFAT_EmitsRepairableError)
{
    // Arrange
    g_proc.outs = { QByteArray("/dev/sdz1: DOS/MBR boot sector FAT (32 bit)") };
    g_proc.exits = { 0 };
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *name) -> bool {
        if (name)
            *name = "UT Disk";
        return true;
    });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "/dev/sdz1"; });
    stub.set_lamda(&UsbRepairMonitor::isHardwareReadOnly,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });

    // Act
    monitor.checkMissingFilesystem("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_EQ(args.at(2).toString(), "vfat");
    EXPECT_EQ(args.at(3).toString(), Defines::kErrorTypeMountFailed);
    EXPECT_TRUE(args.at(4).toBool());
}

TEST_F(UsbRepairServiceTest, CheckMissingFilesystem_FileCmdUnknown_EmitsUnknownError)
{
    // Arrange
    g_proc.outs = { QByteArray("/dev/sdz1: data") };
    g_proc.exits = { 0 };
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *name) -> bool {
        if (name)
            *name = "UT Disk";
        return true;
    });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "/dev/sdz1"; });

    // Act
    monitor.checkMissingFilesystem("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(2).toString(), "unknown");
}

TEST_F(UsbRepairServiceTest, CheckMissingFilesystem_FileCmdFails_NoSignal)
{
    // Arrange
    g_proc.outs = { QByteArray("") };
    g_proc.exits = { 1 };
    UsbRepairMonitor monitor;
    QSignalSpy spy(&monitor, &UsbRepairMonitor::errorDetected);
    stub.set_lamda(&UsbRepairMonitor::shouldIgnoreDevice,
                   [](UsbRepairMonitor *, const QString &) -> bool { return false; });
    stub.set_lamda(&UsbRepairMonitor::isUsbDevice,
                   [](UsbRepairMonitor *, const QString &, QString *) -> bool { return true; });
    stub.set_lamda(&UsbRepairMonitor::getDeviceFile,
                   [](UsbRepairMonitor *, const QString &) -> QString { return "/dev/sdz1"; });

    // Act
    monitor.checkMissingFilesystem("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UsbRepairServiceTest, ShouldIgnoreDevice_InvalidInterface_ReturnsFalse)
{
    // Arrange
    UsbRepairMonitor monitor;   // g_ifaceValid = false

    // Act
    const bool ignored = monitor.shouldIgnoreDevice("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_FALSE(ignored);
    EXPECT_EQ(g_propertyCalls, 0);
}

TEST_F(UsbRepairServiceTest, ShouldIgnoreDevice_HintIgnoreTrue_ReturnsTrue)
{
    // Arrange
    g_ifaceValid = true;
    g_props.insert("HintIgnore", true);
    UsbRepairMonitor monitor;

    // Act
    const bool ignored = monitor.shouldIgnoreDevice("/org/freedesktop/UDisks2/block_devices/sdz1");

    // Assert
    EXPECT_TRUE(ignored);
    EXPECT_GT(g_propertyCalls, 0);
}

/******************************************************************************/
/***************************** UsbRepairWorker ********************************/
/******************************************************************************/

TEST_F(UsbRepairServiceTest, Worker_InitialNotRepairing)
{
    // Arrange
    UsbRepairWorker worker;

    // Act
    const bool repairing = worker.isRepairing("/dev/sdz1");

    // Assert
    EXPECT_FALSE(repairing);
}

TEST_F(UsbRepairServiceTest, StartRepair_InvalidPaths_Rejected)
{
    // Arrange
    UsbRepairWorker worker;
    QString err;

    // Act
    const bool r1 = worker.startRepair("", "ut-caller", err);
    const bool r2 = worker.startRepair("/home/ut/not-dev", "ut-caller", err);
    const bool r3 = worker.startRepair("/dev/../etc/passwd", "ut-caller", err);
    const bool r4 = worker.startRepair("/dev/ut-no-such-device", "ut-caller", err);

    // Assert
    EXPECT_FALSE(r1);
    EXPECT_FALSE(r2);
    EXPECT_FALSE(r3);
    EXPECT_FALSE(r4);
    EXPECT_FALSE(err.isEmpty());
    EXPECT_EQ(g_proc.startCalls, 0);
}

TEST_F(UsbRepairServiceTest, StartRepair_NotUsbBlockDevice_Rejected)
{
    // Arrange
    QTemporaryFile devFile("/dev/shm/ut-usbrepair-XXXXXX");
    ASSERT_TRUE(devFile.open());
    UsbRepairWorker worker;
    stub.set_lamda(&UsbRepairWorker::isUsbBlockDevice,
                   [](UsbRepairWorker *, const QString &) -> bool { return false; });
    QString err;

    // Act
    const bool ok = worker.startRepair(devFile.fileName(), "ut-caller", err);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_FALSE(err.isEmpty());
}

TEST_F(UsbRepairServiceTest, StartRepair_AlreadyRepairing_Rejected)
{
    // Arrange
    QTemporaryFile devFile("/dev/shm/ut-usbrepair-XXXXXX");
    ASSERT_TRUE(devFile.open());
    UsbRepairWorker worker;
    stub.set_lamda(&UsbRepairWorker::isUsbBlockDevice,
                   [](UsbRepairWorker *, const QString &) -> bool { return true; });
    worker.m_activeRepairs.insert(devFile.fileName());
    QString err;

    // Act
    const bool ok = worker.startRepair(devFile.fileName(), "ut-caller", err);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_TRUE(err.contains("already"));
}

TEST_F(UsbRepairServiceTest, StartRepair_FsTypeUndetectable_Rejected)
{
    // Arrange
    QTemporaryFile devFile("/dev/shm/ut-usbrepair-XXXXXX");
    ASSERT_TRUE(devFile.open());
    UsbRepairWorker worker;
    stub.set_lamda(&UsbRepairWorker::isUsbBlockDevice,
                   [](UsbRepairWorker *, const QString &) -> bool { return true; });
    stub.set_lamda(&UsbRepairWorker::detectFsType,
                   [](UsbRepairWorker *, const QString &) -> QString { return ""; });
    QString err;

    // Act
    const bool ok = worker.startRepair(devFile.fileName(), "ut-caller", err);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_FALSE(err.isEmpty());
}

TEST_F(UsbRepairServiceTest, StartRepair_UnsupportedFs_Rejected)
{
    // Arrange
    QTemporaryFile devFile("/dev/shm/ut-usbrepair-XXXXXX");
    ASSERT_TRUE(devFile.open());
    UsbRepairWorker worker;
    stub.set_lamda(&UsbRepairWorker::isUsbBlockDevice,
                   [](UsbRepairWorker *, const QString &) -> bool { return true; });
    stub.set_lamda(&UsbRepairWorker::detectFsType,
                   [](UsbRepairWorker *, const QString &) -> QString { return "zfs"; });
    QString err;

    // Act
    const bool ok = worker.startRepair(devFile.fileName(), "ut-caller", err);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_TRUE(err.contains("Unsupported"));
}

TEST_F(UsbRepairServiceTest, StartRepair_HardwareReadOnly_Rejected)
{
    // Arrange
    QTemporaryFile devFile("/dev/shm/ut-usbrepair-XXXXXX");
    ASSERT_TRUE(devFile.open());
    UsbRepairWorker worker;
    stub.set_lamda(&UsbRepairWorker::isUsbBlockDevice,
                   [](UsbRepairWorker *, const QString &) -> bool { return true; });
    stub.set_lamda(&UsbRepairWorker::detectFsType,
                   [](UsbRepairWorker *, const QString &) -> QString { return "vfat"; });
    g_proc.outs = { QByteArray("1") };   // blockdev --getro => 1
    g_proc.exits = { 0 };
    QString err;

    // Act
    const bool ok = worker.startRepair(devFile.fileName(), "ut-caller", err);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_TRUE(err.contains("write-protected"));
}

TEST_F(UsbRepairServiceTest, StartRepair_AuthFailed_Rejected)
{
    // Arrange
    QTemporaryFile devFile("/dev/shm/ut-usbrepair-XXXXXX");
    ASSERT_TRUE(devFile.open());
    UsbRepairWorker worker;
    stub.set_lamda(&UsbRepairWorker::isUsbBlockDevice,
                   [](UsbRepairWorker *, const QString &) -> bool { return true; });
    stub.set_lamda(&UsbRepairWorker::detectFsType,
                   [](UsbRepairWorker *, const QString &) -> QString { return "vfat"; });
    g_proc.outs = { QByteArray("0") };
    g_proc.exits = { 0 };
    setupPolkit(false);
    QString err;

    // Act
    const bool ok = worker.startRepair(devFile.fileName(), "ut-caller", err);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_TRUE(err.contains("Authorization"));
}

TEST_F(UsbRepairServiceTest, StartRepair_MountedUmountFails_Rejected)
{
    // Arrange
    QTemporaryFile devFile("/dev/shm/ut-usbrepair-XXXXXX");
    ASSERT_TRUE(devFile.open());
    const QString devPath = devFile.fileName();
    UsbRepairWorker worker;
    stub.set_lamda(&UsbRepairWorker::isUsbBlockDevice,
                   [](UsbRepairWorker *, const QString &) -> bool { return true; });
    stub.set_lamda(&UsbRepairWorker::detectFsType,
                   [](UsbRepairWorker *, const QString &) -> QString { return "vfat"; });
    setupPolkit(true);
    // 让 /proc/mounts 读到该设备已挂载
    stubDlsym<bool (*)(QFile *, QIODevice::OpenMode)>(
                   stub, "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE",
                   [devPath](QFile *, QIODevice::OpenMode) -> bool {
        Q_UNUSED(devPath);
        return true;
    });
    static QString s_dev;
    s_dev = devPath;
    stub.set_lamda(static_cast<QByteArray (QIODevice::*)()>(&QIODevice::readAll),
                   []() -> QByteArray {
        return (s_dev + " /media/ut/UDISK vfat rw 0 0\n").toUtf8();
    });
    g_proc.exits = { 1 };   // umount 失败
    QString err;

    // Act
    const bool ok = worker.startRepair(devPath, "ut-caller", err);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_TRUE(err.contains("unmount"));
}

TEST_F(UsbRepairServiceTest, StartRepair_FullSuccess_ExecutesFsckAndFinishes)
{
    // Arrange
    QTemporaryFile devFile("/dev/shm/ut-usbrepair-XXXXXX");
    ASSERT_TRUE(devFile.open());
    const QString devPath = devFile.fileName();
    UsbRepairWorker worker;
    stub.set_lamda(&UsbRepairWorker::isUsbBlockDevice,
                   [](UsbRepairWorker *, const QString &) -> bool { return true; });
    stub.set_lamda(&UsbRepairWorker::detectFsType,
                   [](UsbRepairWorker *, const QString &) -> QString { return "vfat"; });
    g_proc.outs = { QByteArray("0") };
    g_proc.exits = { 0 };
    setupPolkit(true);
    QSignalSpy finSpy(&worker, &UsbRepairWorker::finished);
    QString err;

    // Act
    const bool ok = worker.startRepair(devPath, "ut-caller", err);
    const bool repairing = worker.isRepairing(devPath);
    worker.onFsckFinished(0, QProcess::NormalExit);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_TRUE(err.isEmpty());
    EXPECT_TRUE(repairing);
    ASSERT_EQ(finSpy.count(), 1);
    EXPECT_TRUE(finSpy.takeFirst().at(1).toBool());
    EXPECT_FALSE(worker.isRepairing(devPath));
    EXPECT_EQ(worker.m_currentProcess, nullptr);
}

TEST_F(UsbRepairServiceTest, CancelRepair_AuthFailed_ReturnsFalse)
{
    // Arrange
    UsbRepairWorker worker;
    setupPolkit(false);

    // Act
    const bool ok = worker.cancelRepair("/dev/sdz1", "ut-caller");

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_proc.killCalls, 0);
}

TEST_F(UsbRepairServiceTest, CancelRepair_NoCurrentTask_ReturnsFalse)
{
    // Arrange
    UsbRepairWorker worker;
    setupPolkit(true);

    // Act
    const bool ok = worker.cancelRepair("/dev/sdz1", "ut-caller");

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_proc.killCalls, 0);
}

TEST_F(UsbRepairServiceTest, CancelRepair_MatchingTask_KillsProcess)
{
    // Arrange
    UsbRepairWorker worker;
    setupPolkit(true);
    worker.m_currentDevice = "/dev/sdz1";
    worker.m_currentProcess = new QProcess();

    // Act
    const bool ok = worker.cancelRepair("/dev/sdz1", "ut-caller");

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(g_proc.killCalls, 1);
    delete worker.m_currentProcess;
    worker.m_currentProcess = nullptr;
}

TEST_F(UsbRepairServiceTest, OnFsckReadyRead_ParsesPercentAndEmitsProgress)
{
    // Arrange
    UsbRepairWorker worker;
    worker.m_currentDevice = "/dev/sdz1";
    worker.m_currentProcess = new QProcess();
    QSignalSpy spy(&worker, &UsbRepairWorker::progress);
    g_proc.outs = { QByteArray(" 50% done\nstep two\n") };
    g_proc.errs = { QByteArray("") };

    // Act
    worker.onFsckReadyRead();

    // Assert
    ASSERT_EQ(spy.count(), 2);
    EXPECT_EQ(spy.at(0).at(1).toInt(), 50);
    EXPECT_EQ(spy.at(1).at(1).toInt(), -1);   // 无百分比的行为不确定进度
    EXPECT_TRUE(worker.m_fsckOutput.contains("50%"));
    delete worker.m_currentProcess;
    worker.m_currentProcess = nullptr;
}

TEST_F(UsbRepairServiceTest, OnFsckReadyRead_NoProcess_NoCrash)
{
    // Arrange
    UsbRepairWorker worker;
    QSignalSpy spy(&worker, &UsbRepairWorker::progress);

    // Act
    worker.onFsckReadyRead();

    // Assert
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UsbRepairServiceTest, OnFsckFinished_ExitOne_TreatedAsRepaired)
{
    // Arrange
    UsbRepairWorker worker;
    worker.m_currentDevice = "/dev/sdz1";
    worker.m_currentFsType = "vfat";
    worker.m_activeRepairs.insert("/dev/sdz1");
    worker.m_currentProcess = new QProcess();
    QSignalSpy spy(&worker, &UsbRepairWorker::finished);

    // Act
    worker.onFsckFinished(1, QProcess::NormalExit);

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_TRUE(args.at(1).toBool());
    EXPECT_FALSE(args.at(2).toString().isEmpty());
    EXPECT_FALSE(worker.isRepairing("/dev/sdz1"));
}

TEST_F(UsbRepairServiceTest, OnFsckFinished_NtfsFailure_GivesNtfsHint)
{
    // Arrange
    UsbRepairWorker worker;
    worker.m_currentDevice = "/dev/sdz1";
    worker.m_currentFsType = "ntfs";
    worker.m_activeRepairs.insert("/dev/sdz1");
    worker.m_currentProcess = new QProcess();
    QSignalSpy spy(&worker, &UsbRepairWorker::finished);

    // Act
    worker.onFsckFinished(3, QProcess::NormalExit);

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_FALSE(args.at(1).toBool());
    EXPECT_TRUE(args.at(2).toString().contains("NTFS"));
}

TEST_F(UsbRepairServiceTest, OnFsckFinished_GenericFailure_AppendsOutput)
{
    // Arrange
    UsbRepairWorker worker;
    worker.m_currentDevice = "/dev/sdz1";
    worker.m_currentFsType = "ext4";
    worker.m_activeRepairs.insert("/dev/sdz1");
    worker.m_currentProcess = new QProcess();
    worker.m_fsckOutput = "ut-fsck-failure-detail";
    QSignalSpy spy(&worker, &UsbRepairWorker::finished);

    // Act
    worker.onFsckFinished(4, QProcess::NormalExit);

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_FALSE(args.at(1).toBool());
    EXPECT_TRUE(args.at(2).toString().contains("ut-fsck-failure-detail"));
}

TEST_F(UsbRepairServiceTest, OnFsckFinished_TimedOut_ReportsTimeout)
{
    // Arrange
    UsbRepairWorker worker;
    worker.m_currentDevice = "/dev/sdz1";
    worker.m_currentFsType = "vfat";
    worker.m_activeRepairs.insert("/dev/sdz1");
    worker.m_currentProcess = new QProcess();
    QSignalSpy spy(&worker, &UsbRepairWorker::finished);

    // Act
    worker.onFsckTimeout();   // 设置超时标志并 kill
    worker.onFsckFinished(137, QProcess::CrashExit);

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_FALSE(args.at(1).toBool());
    EXPECT_TRUE(args.at(2).toString().contains("timed out"));
    EXPECT_EQ(g_proc.killCalls, 1);
}

TEST_F(UsbRepairServiceTest, OnFsckFinished_NoProcess_NoSignal)
{
    // Arrange
    UsbRepairWorker worker;
    QSignalSpy spy(&worker, &UsbRepairWorker::finished);

    // Act
    worker.onFsckFinished(0, QProcess::NormalExit);

    // Assert
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UsbRepairServiceTest, OnFsckTimeout_NoProcess_NoCrash)
{
    // Arrange
    UsbRepairWorker worker;

    // Act
    worker.onFsckTimeout();

    // Assert
    EXPECT_FALSE(worker.m_timedOut);
    EXPECT_EQ(g_proc.killCalls, 0);
}

TEST_F(UsbRepairServiceTest, OnFsckErrorOccurred_FailedToStart_CleansUpAndEmits)
{
    // Arrange
    UsbRepairWorker worker;
    worker.m_currentDevice = "/dev/sdz1";
    worker.m_currentFsType = "vfat";
    worker.m_activeRepairs.insert("/dev/sdz1");
    worker.m_currentProcess = new QProcess();
    QSignalSpy spy(&worker, &UsbRepairWorker::finished);

    // Act
    worker.onFsckErrorOccurred(QProcess::FailedToStart);

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_FALSE(args.at(1).toBool());
    EXPECT_EQ(args.at(0).toString(), "/dev/sdz1");
    EXPECT_EQ(worker.m_currentProcess, nullptr);
    EXPECT_FALSE(worker.isRepairing("/dev/sdz1"));
}

TEST_F(UsbRepairServiceTest, OnFsckErrorOccurred_OtherError_Ignored)
{
    // Arrange
    UsbRepairWorker worker;
    QSignalSpy spy(&worker, &UsbRepairWorker::finished);

    // Act
    worker.onFsckErrorOccurred(QProcess::Crashed);

    // Assert
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UsbRepairServiceTest, CheckAuthorization_EmptyCaller_ReturnsFalse)
{
    // Arrange
    UsbRepairWorker worker;
    setupPolkit(true);

    // Act
    const bool ok = worker.checkAuthorization("");

    // Assert
    EXPECT_FALSE(ok);
}

TEST_F(UsbRepairServiceTest, CheckAuthorization_PolkitYesAndNo)
{
    // Arrange
    UsbRepairWorker worker;
    setupPolkit(true);

    // Act
    const bool yes = worker.checkAuthorization("ut-caller");
    g_authYes = 0;
    const bool no = worker.checkAuthorization("ut-caller");

    // Assert
    EXPECT_TRUE(yes);
    EXPECT_FALSE(no);
}

TEST_F(UsbRepairServiceTest, UmountDevice_ExitZero_Succeeds)
{
    // Arrange
    UsbRepairWorker worker;
    g_proc.exits = { 0 };

    // Act
    const bool ok = worker.umountDevice("/dev/sdz1");

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(g_proc.startCalls, 1);
}

TEST_F(UsbRepairServiceTest, UmountDevice_ExitNonZero_Fails)
{
    // Arrange
    UsbRepairWorker worker;
    g_proc.exits = { 1 };
    g_proc.errs = { QByteArray("umount: target is busy") };

    // Act
    const bool ok = worker.umountDevice("/dev/sdz1");

    // Assert
    EXPECT_FALSE(ok);
}

TEST_F(UsbRepairServiceTest, DetectFsType_BlkidExt3_Normalized)
{
    // Arrange
    UsbRepairWorker worker;
    g_proc.outs = { QByteArray("ext3") };
    g_proc.exits = { 0 };

    // Act
    const QString fs = worker.detectFsType("/dev/sdz1");

    // Assert
    EXPECT_EQ(fs, "ext4");
}

TEST_F(UsbRepairServiceTest, DetectFsType_BlkidFails_FallsBackToFileCmd)
{
    // Arrange
    UsbRepairWorker worker;
    g_proc.outs = { QByteArray(""), QByteArray("/dev/sdz1: NTFS file system") };
    g_proc.exits = { 1, 0 };

    // Act
    const QString fs = worker.detectFsType("/dev/sdz1");

    // Assert
    EXPECT_EQ(fs, "ntfs");
    EXPECT_EQ(g_proc.startCalls, 2);
}

TEST_F(UsbRepairServiceTest, DetectFsType_FileCmdExt2Signature_MapsToExt4)
{
    // Arrange
    UsbRepairWorker worker;
    g_proc.outs = { QByteArray(""), QByteArray("/dev/sdz1: ext2 filesystem") };
    g_proc.exits = { 1, 0 };

    // Act
    const QString fs = worker.detectFsType("/dev/sdz1");

    // Assert
    EXPECT_EQ(fs, "ext4");
}

TEST_F(UsbRepairServiceTest, DetectFsType_NoSignature_ReturnsEmpty)
{
    // Arrange
    UsbRepairWorker worker;
    g_proc.outs = { QByteArray(""), QByteArray("/dev/sdz1: data") };
    g_proc.exits = { 1, 0 };

    // Act
    const QString fs = worker.detectFsType("/dev/sdz1");

    // Assert
    EXPECT_TRUE(fs.isEmpty());
}

TEST_F(UsbRepairServiceTest, ValidateDevicePath_AcceptsExistingUsbDevice)
{
    // Arrange
    QTemporaryFile devFile("/dev/shm/ut-usbrepair-XXXXXX");
    ASSERT_TRUE(devFile.open());
    UsbRepairWorker worker;
    stub.set_lamda(&UsbRepairWorker::isUsbBlockDevice,
                   [](UsbRepairWorker *, const QString &) -> bool { return true; });

    // Act
    const bool valid = worker.validateDevicePath(devFile.fileName());

    // Assert
    EXPECT_TRUE(valid);
}

TEST_F(UsbRepairServiceTest, IsUsbBlockDevice_InvalidInterface_ReturnsFalse)
{
    // Arrange
    UsbRepairWorker worker;   // g_ifaceValid = false

    // Act
    const bool usb = worker.isUsbBlockDevice("/dev/sdz1");

    // Assert
    EXPECT_FALSE(usb);
}

TEST_F(UsbRepairServiceTest, IsUsbBlockDevice_RemovableUsbDrive_ReturnsTrue)
{
    // Arrange
    g_ifaceValid = true;
    g_props.insert("Drive", QVariant::fromValue(QDBusObjectPath("/org/freedesktop/UDisks2/drives/UTDrive")));
    g_props.insert("ConnectionBus", "usb");
    g_props.insert("Removable", true);
    UsbRepairWorker worker;

    // Act
    const bool usb = worker.isUsbBlockDevice("/dev/sdz1");

    // Assert
    EXPECT_TRUE(usb);
}

TEST_F(UsbRepairServiceTest, IsUsbBlockDevice_NonRemovable_ReturnsFalse)
{
    // Arrange
    g_ifaceValid = true;
    g_props.insert("Drive", QVariant::fromValue(QDBusObjectPath("/org/freedesktop/UDisks2/drives/UTDrive")));
    g_props.insert("ConnectionBus", "usb");
    g_props.insert("Removable", false);
    UsbRepairWorker worker;

    // Act
    const bool usb = worker.isUsbBlockDevice("/dev/sdz1");

    // Assert
    EXPECT_FALSE(usb);
}

TEST_F(UsbRepairServiceTest, BlockObjPathFromDevice_MapsToUdisksPath)
{
    // Arrange
    UsbRepairWorker worker;

    // Act
    const QString objPath = worker.blockObjPathFromDevice("/dev/sdb1");

    // Assert
    EXPECT_EQ(objPath, "/org/freedesktop/UDisks2/block_devices/sdb1");
}

TEST_F(UsbRepairServiceTest, ExecuteFsck_UnsupportedFs_EmitsFailureAndCleansState)
{
    // Arrange
    UsbRepairWorker worker;
    worker.m_activeRepairs.insert("/dev/sdz1");
    worker.m_currentDevice = "/dev/sdz1";
    QSignalSpy spy(&worker, &UsbRepairWorker::finished);

    // Act
    worker.executeFsck("/dev/sdz1", "zfs");

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_FALSE(args.at(1).toBool());
    EXPECT_TRUE(args.at(2).toString().contains("Unsupported"));
    EXPECT_EQ(worker.m_currentProcess, nullptr);
    EXPECT_FALSE(worker.isRepairing("/dev/sdz1"));
    EXPECT_EQ(g_proc.startCalls, 0);
}
