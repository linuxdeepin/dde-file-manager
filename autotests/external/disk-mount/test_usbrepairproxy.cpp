// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_usbrepairproxy.cpp - UsbRepairProxy 单元测试
// 真实 UsbRepair 服务位于系统总线（测试环境不可用），因此：
//  - 服务缺席路径（m_interface 为空 / 无效）直接验证；
//  - 有效接口路径通过在【私有总线】（test_mock_dbus_env，会话总线桩）上注册
//    同名 mock 服务来构造（asyncCall 不阻塞，同线程注册无死锁风险）。

#include <gtest/gtest.h>

#include <QApplication>
#include <QSignalSpy>
#include <QtTest>
#include <QDBusConnection>
#include <QDBusAbstractInterface>
#include <QDBusPendingCall>
#include <QDBusServiceWatcher>
#include <QVariantMap>

#include "stubext.h"

#include "test_mock_dbus_env.h"
#include "usbrepairproxy.h"

namespace {

// 会话总线上的 mock UsbRepair 服务（与真实系统总线服务同名、同路径、同接口）
class MockUsbRepairService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.UsbRepair")

public:
    QStringList repairPaths;
    QStringList cancelPaths;

public Q_SLOTS:
    bool Repair(const QString &devicePath)
    {
        repairPaths << devicePath;
        return true;
    }
    bool CancelRepair(const QString &devicePath)
    {
        cancelPaths << devicePath;
        return true;
    }

Q_SIGNALS:
    void FsErrorDetected(const QString &devicePath, const QString &deviceName,
                         const QString &fsType, const QString &errorType,
                         bool canRepair, const QString &message);
    void FsErrorCleared(const QString &devicePath);
    void RepairProgress(const QString &devicePath, int percent, const QString &logLine);
    void RepairFinished(const QString &devicePath, bool success, const QString &summary);
};

}   // namespace

class UsbRepairProxyTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        ASSERT_TRUE(ut::ensurePrivateBus()) << "私有 DBus 总线启动失败";
        ut::installSessionBusStub(stub);
    }

    void TearDown() override
    {
        if (serviceRegistered) {
            QDBusConnection::sessionBus().unregisterObject(
                    QStringLiteral("/org/deepin/Filemanager/UsbRepair"));
            QDBusConnection::sessionBus().unregisterService(
                    QStringLiteral("org.deepin.Filemanager.UsbRepair"));
            serviceRegistered = false;
        }
        stub.clear();
        delete proxy;
        proxy = nullptr;
    }

    // 在会话总线上注册 mock 服务，并让代理持有一个指向它的有效接口
    bool attachMockInterface()
    {
        if (!QDBusConnection::sessionBus().registerService(
                    QStringLiteral("org.deepin.Filemanager.UsbRepair"))) {
            return false;
        }
        if (!QDBusConnection::sessionBus().registerObject(
                    QStringLiteral("/org/deepin/Filemanager/UsbRepair"), &mockSvc,
                    QDBusConnection::ExportAllContents)) {
            QDBusConnection::sessionBus().unregisterService(
                    QStringLiteral("org.deepin.Filemanager.UsbRepair"));
            return false;
        }
        serviceRegistered = true;

        proxy->m_interface.reset(new UsbRepairProxy::Interface(
                QStringLiteral("org.deepin.Filemanager.UsbRepair"),
                QStringLiteral("/org/deepin/Filemanager/UsbRepair"),
                QDBusConnection::sessionBus(), proxy));
        return true;
    }

    stub_ext::StubExt stub;
    UsbRepairProxy *proxy { nullptr };
    MockUsbRepairService mockSvc;
    bool serviceRegistered { false };
};

// 构造：系统总线无服务 → watcher 建立；接口无效被复位
TEST_F(UsbRepairProxyTest, Construct_NoSystemBusService_WatcherReadyInterfaceDropped)
{
    // Arrange / Act
    proxy = new UsbRepairProxy();

    // Assert
    EXPECT_GE(proxy->findChildren<QDBusServiceWatcher *>().count(), 1);
    // 服务未在系统总线注册时，构造中的连接尝试失败 → 接口复位为空
    const bool svcRunning = QDBusConnection::systemBus().isConnected()
            && QDBusConnection::systemBus().interface()->isServiceRegistered(
                       QStringLiteral("org.deepin.Filemanager.UsbRepair"));
    if (!svcRunning)
        EXPECT_TRUE(proxy->m_interface.isNull());
    else
        EXPECT_FALSE(proxy->m_interface.isNull());

    // Act / Assert：接口为空时 start/cancel 直接早退（无 DBus 调用）
    proxy->onServiceUnregistered(QStringLiteral("org.deepin.Filemanager.UsbRepair"));
    EXPECT_TRUE(proxy->m_interface.isNull());
    proxy->startRepair(QStringLiteral("/dev/null0"));
    proxy->cancelRepair(QStringLiteral("/dev/null0"));
    SUCCEED() << "null interface path returns without crash";
}

// onServiceRegistered：已有接口 → 早退不重建
TEST_F(UsbRepairProxyTest, OnServiceRegistered_ExistingInterface_EarlyReturns)
{
    // Arrange
    proxy = new UsbRepairProxy();
    ASSERT_TRUE(attachMockInterface());
    auto *before = proxy->m_interface.data();
    EXPECT_TRUE(before->isValid());

    // Act
    proxy->onServiceRegistered(QStringLiteral("org.deepin.Filemanager.UsbRepair"));

    // Assert：指针未变（未重建）
    EXPECT_EQ(proxy->m_interface.data(), before);
}

// onServiceRegistered：接口为空 → 尝试在系统总线重建（无服务时复位）
TEST_F(UsbRepairProxyTest, OnServiceRegistered_NoInterface_RecreatesOnSystemBus)
{
    // Arrange
    proxy = new UsbRepairProxy();
    proxy->onServiceUnregistered(QStringLiteral("x"));   // 确保置空
    ASSERT_TRUE(proxy->m_interface.isNull());

    // Act
    proxy->onServiceRegistered(QStringLiteral("org.deepin.Filemanager.UsbRepair"));

    // Assert：构造尝试发生（无崩溃）；随后注销处理再次置空
    proxy->onServiceUnregistered(QStringLiteral("org.deepin.Filemanager.UsbRepair"));
    EXPECT_TRUE(proxy->m_interface.isNull());
}

// startRepair / cancelRepair：有效接口（会话总线 mock）→ 发出对应 DBus 调用
TEST_F(UsbRepairProxyTest, StartAndCancelRepair_ValidInterface_CallsServiceMethods)
{
    // Arrange
    proxy = new UsbRepairProxy();
    ASSERT_TRUE(attachMockInterface());
    proxy->connectSignals();

    // Act
    proxy->startRepair(QStringLiteral("/dev/utrepair0"));
    proxy->cancelRepair(QStringLiteral("/dev/utrepair1"));
    QTest::qWait(200);

    // Assert
    ASSERT_EQ(mockSvc.repairPaths.count(), 1);
    EXPECT_EQ(mockSvc.repairPaths.first(), QString("/dev/utrepair0"));
    ASSERT_EQ(mockSvc.cancelPaths.count(), 1);
    EXPECT_EQ(mockSvc.cancelPaths.first(), QString("/dev/utrepair1"));
}

// connectSignals：mock 服务的信号被转发为代理信号
TEST_F(UsbRepairProxyTest, ConnectSignals_ServiceSignals_RelayedByProxy)
{
    // Arrange
    proxy = new UsbRepairProxy();
    ASSERT_TRUE(attachMockInterface());
    proxy->connectSignals();

    QSignalSpy errSpy(proxy, &UsbRepairProxy::fsErrorDetected);
    QSignalSpy clrSpy(proxy, &UsbRepairProxy::fsErrorCleared);
    QSignalSpy progSpy(proxy, &UsbRepairProxy::repairProgress);
    QSignalSpy finSpy(proxy, &UsbRepairProxy::repairFinished);

    // Act
    Q_EMIT mockSvc.FsErrorDetected("/dev/ut0", "UT", "vfat", "dirty", true, "m");
    Q_EMIT mockSvc.FsErrorCleared("/dev/ut0");
    Q_EMIT mockSvc.RepairProgress("/dev/ut0", 33, "log");
    Q_EMIT mockSvc.RepairFinished("/dev/ut0", true, "ok");
    QTest::qWait(300);

    // Assert
    EXPECT_EQ(errSpy.count(), 1);
    EXPECT_EQ(clrSpy.count(), 1);
    EXPECT_EQ(progSpy.count(), 1);
    EXPECT_EQ(finSpy.count(), 1);
    ASSERT_EQ(errSpy.count(), 1);
    EXPECT_EQ(errSpy.first().at(0).toString(), QString("/dev/ut0"));
    EXPECT_EQ(errSpy.first().at(1).toString(), QString("UT"));
    EXPECT_EQ(finSpy.first().at(2).toString(), QString("ok"));
}

// 析构：父子关系正确，删除无泄漏崩溃
TEST_F(UsbRepairProxyTest, Destructor_DeletesWatcherAndInterface)
{
    // Arrange
    proxy = new UsbRepairProxy();
    ASSERT_TRUE(attachMockInterface());
    const QDBusServiceWatcher *watcher = proxy->findChildren<QDBusServiceWatcher *>().constFirst();
    EXPECT_NE(watcher, nullptr);

    // Act / Assert
    delete proxy;
    proxy = nullptr;
    SUCCEED() << "destroyed without crash";
}

#include "test_usbrepairproxy.moc"
