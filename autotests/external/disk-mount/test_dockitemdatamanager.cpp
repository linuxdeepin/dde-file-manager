// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_dockitemdatamanager.cpp - disk-mount 插件 DockItemDataManager 单元测试
// 源码随测试目标一起编译；DBus 走 test_mock_dbus_env 提供的私有总线 mock 服务
// （DeviceManager / Notifications），见 test_mock_dbus_env.h 头注释。
//
// 约定：未知块设备路径统一回 kDrive="UTROOT"，isRootDrive() 的 call_once
// 缓存无论首次触发于哪条用例，rootDrive 恒为 "UTROOT"，用例间无顺序依赖。
//
// 分支映射（dockitemdatamanager.cpp）：
// onBlockMounted/onBlockUnmounted/onBlockPropertyChanged → 设备增删
// blockDeviceFilter 各拒绝/放行分支、protoDeviceFilter dlnfs 分支
// isRootDrive + lambda / isMountPointInFstab（真实 /etc/fstab）
// playSoundOnDevPlugInOut（桩 DDesktopServices 声音 + mock Notify 记录）
// sendNotification / notify / notifyWithActions / closeNotification
// onServiceRegistered / onServiceUnregistered / updateDockVisible
// initialize 正常与两处错误早退；ejectAll / ejectDevice 分发
// connectDeviceManger 的 4 个 lambda（直接 emit 代理信号）
// watchService 的 2 个 lambda（emit QDBusServiceWatcher 信号）
// subscribe/unsubscribe/refreshUsage（QTimer::singleShot 异步校验）
// onFsErrorDetected 三分支 / onFsErrorCleared / onNotifyActionInvoked /
// onNotifyClosed / onRepairProgress / onRepairFinished 全分支

#include <gtest/gtest.h>

#include <QApplication>
#include <QSignalSpy>
#include <QtTest>
#include <QDBusAbstractInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusServiceWatcher>
#include <QVariantMap>
#include <QProcess>
#include <QFile>

#include <fstab.h>
#include <sys/stat.h>
#include <cstring>

#include <DDesktopServices>

#include "stubext.h"

#include "test_mock_dbus_env.h"
#include "dockitemdatamanager.h"
#include "dockutils.h"
#include "usbrepairproxy.h"
#include "widgets/repairdialog.h"
#include "global_server_defines.h"

using namespace GlobalServerDefines;
using namespace Dtk::Gui;

namespace {

QVariantMap makeBlockData()
{
    return {
        { DeviceProperty::kId, QString("utblk") },
        { DeviceProperty::kHintIgnore, false },
        { DeviceProperty::kMountPoint, QString("/media/utblk") },
        { DeviceProperty::kOpticalDrive, false },
        { DeviceProperty::kDrive, QString("/org/freedesktop/UDisks2/drives/utdrive") },
        { DeviceProperty::kCryptoBackingDevice, QString("/") },
        { DeviceProperty::kConnectionBus, QString("usb") },
        { DeviceProperty::kIdLabel, QString("UT-Disk") },
        { DeviceProperty::kSizeTotal, 2048ULL * 1024 * 1024 },
        { DeviceProperty::kSizeUsed, 1024ULL * 1024 * 1024 },
    };
}

QVariantMap makeProtoData()
{
    return {
        { DeviceProperty::kId, QString(ut::kProtoId) },
        { DeviceProperty::kMountPoint, QString("/media/utproto") },
        { DeviceProperty::kDisplayName, QString("net-share") },
        { DeviceProperty::kSizeTotal, 4096ULL },
        { DeviceProperty::kSizeUsed, 512ULL },
    };
}

}   // namespace

class DockItemDataManagerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        ASSERT_TRUE(ut::ensurePrivateBus()) << "私有 DBus 总线启动失败";
        ut::installSessionBusStub(stub);
        ut::devMngMock().reset();
        ut::notifyMock().reset();
        ut::devMngMock().blockData = makeBlockData();
        ut::devMngMock().protoData = makeProtoData();
        qRegisterMetaType<DockItemData>("DockItemData");
        mgr = new DockItemDataManager();
    }

    void TearDown() override
    {
        stub.clear();
        delete mgr;
        mgr = nullptr;
    }

    stub_ext::StubExt stub;
    DockItemDataManager *mgr { nullptr };
};

// instance() 单例：两次调用返回同一指针
TEST_F(DockItemDataManagerTest, Instance_Singleton_ReturnsSamePointer)
{
    // Arrange / Act
    DockItemDataManager *first = DockItemDataManager::instance();
    DockItemDataManager *second = DockItemDataManager::instance();

    // Assert
    EXPECT_EQ(first, second);
    EXPECT_NE(first, nullptr);
    EXPECT_NE(first, mgr);   // 直接构造的对象与单例相互独立
}

// onBlockMounted：过滤通过 → blocks 入表 + mountAdded + 可见性 true
TEST_F(DockItemDataManagerTest, OnBlockMounted_ValidDevice_EmitsMountAddedAndVisible)
{
    // Arrange
    QSignalSpy added(mgr, &DockItemDataManager::mountAdded);
    QSignalSpy visible(mgr, &DockItemDataManager::requesetSetDockVisible);

    // Act
    mgr->onBlockMounted(QString(ut::kBlockId));

    // Assert
    EXPECT_EQ(added.count(), 1);
    EXPECT_TRUE(mgr->blocks.contains(QString(ut::kBlockId)));
    EXPECT_EQ(visible.count(), 1);
    EXPECT_TRUE(visible.first().first().toBool());
    const DockItemData item = mgr->blocks.value(QString(ut::kBlockId));
    EXPECT_EQ(item.displayName, QString("UT-Disk"));
    EXPECT_FALSE(item.isProtocolDevice);
    EXPECT_EQ(item.id, QString("utblk"));
    EXPECT_EQ(item.backingID, QString("utblk"));   // crypto "/" → 用自身 id
    EXPECT_EQ(item.targetUrl.toLocalFile(), QString("/media/utblk"));
    EXPECT_TRUE(item.sortKey.startsWith("00"));
    EXPECT_EQ(item.totalSize, 2048ULL * 1024 * 1024);
    EXPECT_EQ(item.usedSize, 1024ULL * 1024 * 1024);
}

// onBlockMounted：数据被过滤器拒绝 → 不发信号
TEST_F(DockItemDataManagerTest, OnBlockMounted_FilterRejected_NoSignal)
{
    // Arrange：HintIgnore=true 被拒
    auto data = makeBlockData();
    data[DeviceProperty::kHintIgnore] = true;
    ut::devMngMock().blockData = data;
    QSignalSpy added(mgr, &DockItemDataManager::mountAdded);

    // Act
    mgr->onBlockMounted(QString(ut::kBlockId));

    // Assert
    EXPECT_EQ(added.count(), 0);
    EXPECT_TRUE(mgr->blocks.isEmpty());
}

// onBlockUnmounted：不在表中 → 忽略；在表中 → 移除 + mountRemoved + 不可见
TEST_F(DockItemDataManagerTest, OnBlockUnmounted_KnownAndUnknownId_RemovesAndEmits)
{
    // Arrange
    mgr->blocks.insert(QString(ut::kBlockId), DockItemData { .id = "utblk" });
    QSignalSpy removed(mgr, &DockItemDataManager::mountRemoved);
    QSignalSpy visible(mgr, &DockItemDataManager::requesetSetDockVisible);

    // Act：未知 id 先来
    mgr->onBlockUnmounted(QString("/unknown"));
    EXPECT_EQ(removed.count(), 0);
    EXPECT_EQ(visible.count(), 0);

    // Act：已知 id
    mgr->onBlockUnmounted(QString(ut::kBlockId));

    // Assert
    EXPECT_EQ(removed.count(), 1);
    EXPECT_EQ(removed.first().first().toString(), QString(ut::kBlockId));
    EXPECT_FALSE(mgr->blocks.contains(QString(ut::kBlockId)));
    ASSERT_EQ(visible.count(), 1);
    EXPECT_FALSE(visible.first().first().toBool());
}

// onBlockPropertyChanged：HintIgnore 置位/复位 → 卸载/挂载
TEST_F(DockItemDataManagerTest, OnBlockPropertyChanged_HintIgnore_TogglesMount)
{
    // Arrange
    QSignalSpy added(mgr, &DockItemDataManager::mountAdded);
    QSignalSpy removed(mgr, &DockItemDataManager::mountRemoved);

    // Act / Assert：false → onBlockMounted
    mgr->onBlockPropertyChanged(QString(ut::kBlockId), "HintIgnore", QDBusVariant(false));
    EXPECT_EQ(added.count(), 1);
    EXPECT_TRUE(mgr->blocks.contains(QString(ut::kBlockId)));

    // Act / Assert：true → onBlockUnmounted
    mgr->onBlockPropertyChanged(QString(ut::kBlockId), "HintIgnore", QDBusVariant(true));
    EXPECT_EQ(removed.count(), 1);
    EXPECT_FALSE(mgr->blocks.contains(QString(ut::kBlockId)));

    // 无关属性不应产生信号
    mgr->onBlockPropertyChanged(QString(ut::kBlockId), "OtherProp", QDBusVariant(1));
    EXPECT_EQ(added.count(), 1);
    EXPECT_EQ(removed.count(), 1);
}

// onBlockPropertyChanged：光驱介质被物理弹出 → 卸载
TEST_F(DockItemDataManagerTest, OnBlockPropertyChanged_OpticalMediaEjected_Unmounts)
{
    // Arrange
    const QString sr0 { "/org/freedesktop/UDisks2/block_devices/sr0" };
    mgr->blocks.insert(sr0, DockItemData { .id = "sr0" });
    QSignalSpy removed(mgr, &DockItemDataManager::mountRemoved);

    // Act
    mgr->onBlockPropertyChanged(sr0, QString(DeviceProperty::kMediaAvailable), QDBusVariant(false));

    // Assert
    EXPECT_EQ(removed.count(), 1);
    EXPECT_FALSE(mgr->blocks.contains(sr0));
}

// onProtocolMounted / onProtocolUnmounted：协议设备增删
TEST_F(DockItemDataManagerTest, OnProtocolMountedAndUnmounted_EmitsAndTracks)
{
    // Arrange
    QSignalSpy added(mgr, &DockItemDataManager::mountAdded);
    QSignalSpy removed(mgr, &DockItemDataManager::mountRemoved);
    QSignalSpy visible(mgr, &DockItemDataManager::requesetSetDockVisible);

    // Act
    mgr->onProtocolMounted(QString(ut::kProtoId));

    // Assert
    EXPECT_EQ(added.count(), 1);
    EXPECT_TRUE(mgr->protocols.contains(QString(ut::kProtoId)));
    const DockItemData item = mgr->protocols.value(QString(ut::kProtoId));
    EXPECT_TRUE(item.isProtocolDevice);
    EXPECT_EQ(item.displayName, QString("net-share"));
    EXPECT_EQ(item.backingID, QString(ut::kProtoId));
    EXPECT_EQ(item.targetFileUrl.toLocalFile(), QString("/media/utproto"));
    EXPECT_TRUE(item.sortKey.startsWith("01"));

    // Act：卸载
    mgr->onProtocolUnmounted(QString(ut::kProtoId));

    // Assert
    EXPECT_EQ(removed.count(), 1);
    EXPECT_FALSE(mgr->protocols.contains(QString(ut::kProtoId)));
    ASSERT_GE(visible.count(), 2);
    EXPECT_FALSE(visible.last().first().toBool());
}

// blockDeviceFilter：各拒绝分支与放行分支
TEST_F(DockItemDataManagerTest, BlockDeviceFilter_AllBranches_DecidesCorrectly)
{
    // Act / Assert：HintIgnore → 拒绝
    auto hintIgnored = makeBlockData();
    hintIgnored[DeviceProperty::kHintIgnore] = true;
    EXPECT_FALSE(mgr->blockDeviceFilter(hintIgnored));

    // 空挂载点 → 拒绝
    auto noMpt = makeBlockData();
    noMpt[DeviceProperty::kMountPoint] = "";
    EXPECT_FALSE(mgr->blockDeviceFilter(noMpt));

    // fstab 挂载点 → 拒绝（成员方法打桩保证确定性：真实实现依赖宿主机
    // /etc/fstab 的内容与 mtime，本环境 mtime=0 命中源码缓存分支恒返回 false）
    stub.set_lamda(&DockItemDataManager::isMountPointInFstab,
                   [](DockItemDataManager *, const QString &mpt) -> bool {
        __DBG_STUB_INVOKE__
        return mpt == QStringLiteral("/media/ut-fstab");
    });
    auto fstabMpt = makeBlockData();
    fstabMpt[DeviceProperty::kMountPoint] = "/media/ut-fstab";
    EXPECT_FALSE(mgr->blockDeviceFilter(fstabMpt));

    // 光驱 → 放行（先于根盘判断）
    auto optical = makeBlockData();
    optical[DeviceProperty::kMountPoint] = "/media/cdrom";
    optical[DeviceProperty::kOpticalDrive] = true;
    EXPECT_TRUE(mgr->blockDeviceFilter(optical));

    // 根盘兄弟分区 → 拒绝（isRootDrive 命中，根盘路径来自 mock=UTROOT）
    auto rootSibling = makeBlockData();
    rootSibling[DeviceProperty::kDrive] = QString(ut::kRootDrive);
    EXPECT_FALSE(mgr->blockDeviceFilter(rootSibling));

    // 普通分区 usb → 放行
    EXPECT_TRUE(mgr->blockDeviceFilter(makeBlockData()));

    // 非 usb 总线 → 拒绝
    auto sata = makeBlockData();
    sata[DeviceProperty::kConnectionBus] = "sata";
    EXPECT_FALSE(mgr->blockDeviceFilter(sata));
}

// blockDeviceFilter：加密载体分支（backing 设备 usb / 非 usb）
TEST_F(DockItemDataManagerTest, BlockDeviceFilter_CryptoBacking_ChecksBackingBus)
{
    // Act / Assert：backing 为 usb → 放行
    ut::devMngMock().backingData = { { DeviceProperty::kConnectionBus, QString("usb") } };
    auto encrypted = makeBlockData();
    encrypted[DeviceProperty::kCryptoBackingDevice] = QString(ut::kBackingId);
    EXPECT_TRUE(mgr->blockDeviceFilter(encrypted));
    EXPECT_TRUE(ut::devMngMock().methods.contains("QueryBlockDeviceInfo"));

    // backing 非 usb → 拒绝
    ut::devMngMock().methods.clear();
    ut::devMngMock().backingData = { { DeviceProperty::kConnectionBus, QString("sata") } };
    EXPECT_FALSE(mgr->blockDeviceFilter(encrypted));
}

// isRootDrive：与 call_once 缓存的根盘路径比较
// （once_flag/rootDrive 为函数内静态，进程级共享：首次触发可能位于任意更早
//  的用例——mock 未知路径恒回 kDrive="UTROOT"，故缓存值确定，无顺序依赖）
TEST_F(DockItemDataManagerTest, IsRootDrive_MatchesMockedRootDrive)
{
    // Act
    const bool hit = mgr->isRootDrive(QString(ut::kRootDrive));
    const bool miss = mgr->isRootDrive("/org/freedesktop/UDisks2/drives/other");

    // Assert
    EXPECT_TRUE(hit);
    EXPECT_FALSE(miss);
}

// isMountPointInFstab：真实解析逻辑（stat/getfsent 打桩保证确定性：
// 本环境 /etc/fstab 的 mtime=0 恰与静态缓存初值相等，源码据此跳过解析，
// 真实调用恒返回 false，故用桩驱动完整解析分支，详见缺陷清单）
TEST_F(DockItemDataManagerTest, IsMountPointInFstab_ParsesAndMatchesEntries)
{
    // Arrange：stat 固定返回非零 mtime（越过缓存判断），getfsent 吐一条记录
    static struct fstab utFstabEntry;
    utFstabEntry.fs_spec = const_cast<char *>("/dev/utroot");
    utFstabEntry.fs_file = const_cast<char *>("/ut/fstab/mpt");
    utFstabEntry.fs_vfstype = const_cast<char *>("ext4");
    utFstabEntry.fs_mntops = const_cast<char *>("rw");
    utFstabEntry.fs_type = const_cast<char *>("rw");
    utFstabEntry.fs_freq = 1;
    utFstabEntry.fs_passno = 1;

    int statCalls = 0;
    stub.set_lamda(static_cast<int (*)(const char *, struct stat *)>(&stat),
                   [&statCalls](const char *, struct stat *st) -> int {
        __DBG_STUB_INVOKE__
        memset(st, 0, sizeof(struct stat));
        st->st_mtime = 424242;
        ++statCalls;
        return 0;
    });
    stub.set_lamda(&getfsent, []() -> struct fstab * {
        __DBG_STUB_INVOKE__
        static int calls = 0;
        return calls++ == 0 ? &utFstabEntry : nullptr;
    });

    // Act / Assert
    EXPECT_FALSE(mgr->isMountPointInFstab(""));   // 空串短路（不走 stat）
    EXPECT_TRUE(mgr->isMountPointInFstab("/ut/fstab/mpt"));
    EXPECT_FALSE(mgr->isMountPointInFstab("/no/such/ut/mountpoint"));
    EXPECT_GE(statCalls, 2);   // 两个非空查询均经过缓存检查
}

// protoDeviceFilter：dlnfs 拒绝 + 普通放行
TEST_F(DockItemDataManagerTest, ProtoDeviceFilter_DlnfsRejected_OthersDisplayed)
{
    // Arrange：桩掉 mtab 扫描
    stub.set_lamda(&device_utils::isDlnfsMount,
                   [](const QString &) -> bool { __DBG_STUB_INVOKE__ return true; });

    // Act / Assert
    EXPECT_FALSE(mgr->protoDeviceFilter(makeProtoData()));   // dlnfs → ignore

    stub.set_lamda(&device_utils::isDlnfsMount,
                   [](const QString &) -> bool { __DBG_STUB_INVOKE__ return false; });
    EXPECT_TRUE(mgr->protoDeviceFilter(makeProtoData()));
}

// playSoundOnDevPlugInOut：播放对应音效；拔出时另发通知
TEST_F(DockItemDataManagerTest, PlaySoundOnDevPlugInOut_PlaysEffectAndNotifiesOnRemoval)
{
    // Arrange
    QList<int> effects;
    stub.set_lamda(static_cast<bool (*)(const DDesktopServices::SystemSoundEffect &)>(
                           &DDesktopServices::playSystemSoundEffect),
                   [&effects](const DDesktopServices::SystemSoundEffect &e) -> bool {
        __DBG_STUB_INVOKE__
        effects << static_cast<int>(e);
        return true;
    });

    // Act
    mgr->playSoundOnDevPlugInOut(true);

    // Assert
    ASSERT_EQ(effects.count(), 1);
    EXPECT_EQ(effects.first(), static_cast<int>(DDesktopServices::SSE_DeviceAdded));
    EXPECT_FALSE(ut::notifyMock().methods.contains("Notify"));   // 插入不通知

    // Act：拔出
    mgr->playSoundOnDevPlugInOut(false);

    // Assert
    ASSERT_EQ(effects.count(), 2);
    EXPECT_EQ(effects.last(), static_cast<int>(DDesktopServices::SSE_DeviceRemoved));
    ASSERT_TRUE(ut::notifyMock().methods.contains("Notify"));
    const QVariantList args = ut::notifyMock().args.at(ut::notifyMock().methods.indexOf("Notify"));
    EXPECT_EQ(args.value(3).toString(), QString());   // title 为空
    EXPECT_TRUE(args.value(4).toString().contains("safely removed"));
}

// sendNotification：设备不在表中直接返回；在表中拼装忙碌通知
TEST_F(DockItemDataManagerTest, SendNotification_DeviceBusy_NotifiesWithDeviceName)
{
    // Arrange
    DockItemData item { .id = "utblk", .displayName = "UT-Disk" };
    mgr->blocks.insert(QString(ut::kBlockId), item);

    // Act：未知 id → 无通知
    mgr->sendNotification("/unknown", GlobalServerDefines::DeviceBusyAction::kEject);
    EXPECT_FALSE(ut::notifyMock().methods.contains("Notify"));

    // Act：块设备 unmount 失败
    mgr->sendNotification(QString(ut::kBlockId), GlobalServerDefines::DeviceBusyAction::kUnmount);

    // Assert
    ASSERT_TRUE(ut::notifyMock().methods.contains("Notify"));
    const QVariantList args = ut::notifyMock().args.at(ut::notifyMock().methods.indexOf("Notify"));
    EXPECT_TRUE(args.value(4).toString().contains("UT-Disk"));
    EXPECT_TRUE(args.value(4).toString().contains("unmount"));

    // 协议设备 eject 失败
    mgr->protocols.insert("utproto1", DockItemData { .id = "utproto1", .displayName = "net-share" });
    ut::notifyMock().reset();
    mgr->sendNotification("utproto1", GlobalServerDefines::DeviceBusyAction::kEject);
    ASSERT_TRUE(ut::notifyMock().methods.contains("Notify"));
    EXPECT_TRUE(ut::notifyMock().args.last().value(4).toString().contains("net-share"));

    // 未知动作回退为 "remove" 文案
    ut::notifyMock().reset();
    mgr->sendNotification("utproto1", "unknown-op");
    ASSERT_TRUE(ut::notifyMock().methods.contains("Notify"));
    EXPECT_TRUE(ut::notifyMock().args.last().value(4).toString().contains("remove"));
}

// updateDockVisible：空表 → false；非空 → true
TEST_F(DockItemDataManagerTest, UpdateDockVisible_DependsOnContainers)
{
    // Arrange
    QSignalSpy visible(mgr, &DockItemDataManager::requesetSetDockVisible);

    // Act / Assert：空
    mgr->updateDockVisible();
    ASSERT_EQ(visible.count(), 1);
    EXPECT_FALSE(visible.last().first().toBool());

    // 非空
    mgr->protocols.insert("p1", DockItemData { .id = "p1" });
    mgr->updateDockVisible();
    ASSERT_EQ(visible.count(), 2);
    EXPECT_TRUE(visible.last().first().toBool());
}

// onServiceRegistered：重建代理并 initialize
TEST_F(DockItemDataManagerTest, OnServiceRegistered_ReconnectsAndInitializes)
{
    // Arrange
    ut::devMngMock().blockIds = { QString(ut::kBlockId) };
    ut::devMngMock().protoIds = { QString(ut::kProtoId) };
    QSignalSpy added(mgr, &DockItemDataManager::mountAdded);

    // Act
    mgr->onServiceRegistered();

    // Assert
    EXPECT_EQ(added.count(), 2);
    EXPECT_TRUE(mgr->blocks.contains(QString(ut::kBlockId)));
    EXPECT_TRUE(mgr->protocols.contains(QString(ut::kProtoId)));
}

// onServiceUnregistered：清空并逐个发 mountRemoved
TEST_F(DockItemDataManagerTest, OnServiceUnregistered_ClearsAndEmitsRemovals)
{
    // Arrange
    mgr->blocks.insert("b1", DockItemData { .id = "b1" });
    mgr->protocols.insert("p1", DockItemData { .id = "p1" });
    QSignalSpy removed(mgr, &DockItemDataManager::mountRemoved);
    QSignalSpy visible(mgr, &DockItemDataManager::requesetSetDockVisible);

    // Act
    mgr->onServiceUnregistered();

    // Assert
    EXPECT_EQ(removed.count(), 2);
    EXPECT_TRUE(mgr->blocks.isEmpty());
    EXPECT_TRUE(mgr->protocols.isEmpty());
    ASSERT_EQ(visible.count(), 1);
    EXPECT_FALSE(visible.first().first().toBool());
}

// initialize：块设备列表查询报错 → 早退不崩溃
TEST_F(DockItemDataManagerTest, Initialize_DbusError_ReturnsEarly)
{
    // Arrange
    ut::devMngMock().failBlockList = true;
    QSignalSpy added(mgr, &DockItemDataManager::mountAdded);

    // Act
    mgr->initialize();

    // Assert
    EXPECT_EQ(added.count(), 0);
    EXPECT_TRUE(mgr->blocks.isEmpty());
    EXPECT_TRUE(mgr->protocols.isEmpty());
}

// initialize：协议设备查询报错也早退
TEST_F(DockItemDataManagerTest, Initialize_ProtocolQueryError_StillSafe)
{
    // Arrange：块设备阶段成功（空表），协议列表查询失败
    ut::devMngMock().failProtocolList = true;
    QSignalSpy added(mgr, &DockItemDataManager::mountAdded);

    // Act
    mgr->initialize();

    // Assert：块设备阶段查询过，整体安全早退
    EXPECT_TRUE(ut::devMngMock().methods.contains("GetBlockDevicesIdList"));
    EXPECT_TRUE(ut::devMngMock().methods.contains("GetProtocolDevicesIdList"));
    EXPECT_EQ(added.count(), 0);
    EXPECT_TRUE(mgr->protocols.isEmpty());
}

// ejectAll / ejectDevice：分发到正确的 DBus 方法
TEST_F(DockItemDataManagerTest, EjectAllAndEjectDevice_DispatchCorrectDBusCalls)
{
    // Act
    mgr->ejectAll();
    mgr->ejectDevice("/org/freedesktop/UDisks2/block_devices/sdb1");
    mgr->ejectDevice("gphoto2://usb");

    // Assert
    const auto &m = ut::devMngMock();
    EXPECT_TRUE(m.methods.contains("DetachAllMountedDevices"));
    ASSERT_TRUE(m.methods.contains("DetachBlockDevice"));
    EXPECT_EQ(m.args.at(m.methods.indexOf("DetachBlockDevice")).value(0).toString(),
              QString("/org/freedesktop/UDisks2/block_devices/sdb1"));
    ASSERT_TRUE(m.methods.contains("DetachProtocolDevice"));
    EXPECT_EQ(m.args.at(m.methods.indexOf("DetachProtocolDevice")).value(0).toString(),
              QString("gphoto2://usb"));
}

// connectDeviceManger 的 lambda：BlockDriveAdded/Removed → 音效
TEST_F(DockItemDataManagerTest, ConnectDeviceManager_DriveAddedRemovedLambdas_PlaySounds)
{
    // Arrange
    QList<int> effects;
    stub.set_lamda(static_cast<bool (*)(const DDesktopServices::SystemSoundEffect &)>(
                           &DDesktopServices::playSystemSoundEffect),
                   [&effects](const DDesktopServices::SystemSoundEffect &e) -> bool {
        __DBG_STUB_INVOKE__
        effects << static_cast<int>(e);
        return true;
    });

    // Act：直接触发代理信号（与 DBus 到达路径等价）
    Q_EMIT mgr->devMng.data()->BlockDriveAdded();
    Q_EMIT mgr->devMng.data()->BlockDriveRemoved();

    // Assert
    ASSERT_EQ(effects.count(), 2);
    EXPECT_EQ(effects.at(0), static_cast<int>(DDesktopServices::SSE_DeviceAdded));
    EXPECT_EQ(effects.at(1), static_cast<int>(DDesktopServices::SSE_DeviceRemoved));
    EXPECT_TRUE(ut::notifyMock().methods.contains("Notify"));   // Removed 附带安全移除通知
}

// connectDeviceManger 的 SizeUsedChanged lambda → usageUpdated 信号
TEST_F(DockItemDataManagerTest, ConnectDeviceManager_SizeUsedChangedLambda_EmitsUsage)
{
    // Arrange
    QSignalSpy usage(mgr, &DockItemDataManager::usageUpdated);

    // Act
    Q_EMIT mgr->devMng.data()->SizeUsedChanged("dev1", 100, 30);

    // Assert
    ASSERT_EQ(usage.count(), 1);
    EXPECT_EQ(usage.first().at(0).toString(), QString("dev1"));
    EXPECT_EQ(usage.first().at(1).toULongLong(), 70ULL);   // total - free
}

// watchService 的两个 lambda：emit watcher 信号触发注册/注销处理
// 注意：DeviceManager（QDBusAbstractInterface）内部也会创建监听同一 daemon
// 服务的 watcher（devMng 的子对象），因此对所有监听 daemon 的 watcher 各发
// 一次信号——只有 watchService() 里连接了 lambda 的那个会生效。
TEST_F(DockItemDataManagerTest, WatchService_Lambdas_RouteToRegisteredUnregistered)
{
    // Arrange
    const auto watchers = mgr->findChildren<QDBusServiceWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    const QStringList daemonWatchees { QStringLiteral("org.deepin.Filemanager.Daemon") };
    QSignalSpy visible(mgr, &DockItemDataManager::requesetSetDockVisible);

    // Act：unregistered → 清空
    mgr->blocks.insert("b1", DockItemData { .id = "b1" });
    for (QDBusServiceWatcher *w : watchers) {
        if (w->watchedServices() == daemonWatchees)
            Q_EMIT w->serviceUnregistered("org.deepin.Filemanager.Daemon");
    }
    EXPECT_TRUE(mgr->blocks.isEmpty());
    ASSERT_EQ(visible.count(), 1);
    EXPECT_FALSE(visible.first().first().toBool());

    // Act：registered → 重建 + initialize（mock 返回空表）
    for (QDBusServiceWatcher *w : watchers) {
        if (w->watchedServices() == daemonWatchees)
            Q_EMIT w->serviceRegistered("org.deepin.Filemanager.Daemon");
    }

    // Assert
    EXPECT_TRUE(ut::devMngMock().methods.contains("GetBlockDevicesIdList"));
    EXPECT_EQ(visible.count(), 2);   // initialize 末尾再发一次
}

// subscribe/unsubscribe/refreshUsage：异步 singleShot 中调用 DBus 方法
TEST_F(DockItemDataManagerTest, UsageMonitoring_SubscribeUnsubscribeRefresh_CallDBusAsync)
{
    // Act
    mgr->subscribeUsageMonitoring();
    mgr->unsubscribeUsageMonitoring();
    mgr->refreshUsage();
    QTest::qWait(100);

    // Assert
    const auto &m = ut::devMngMock();
    EXPECT_TRUE(m.methods.contains("StartMonitoringUsage"));
    EXPECT_TRUE(m.methods.contains("StopMonitoringUsage"));
    EXPECT_TRUE(m.methods.contains("RefreshDeviceUsage"));
    EXPECT_EQ(m.methods.count("RefreshDeviceUsage"), 1);
}

// onFsErrorDetected：可修复 → 带按钮的通知并记录待处理错误
TEST_F(DockItemDataManagerTest, OnFsErrorDetected_Repairable_SendsNotificationWithActions)
{
    // Arrange
    ut::notifyMock().notifyId = 88;

    // Act
    mgr->onFsErrorDetected("/dev/utblk", "UT-Disk", "vfat", "dirty", true, "msg");

    // Assert
    ASSERT_TRUE(ut::notifyMock().methods.contains("Notify"));
    const QVariantList args = ut::notifyMock().args.at(ut::notifyMock().methods.indexOf("Notify"));
    EXPECT_TRUE(args.value(5).toStringList().contains("repair"));
    EXPECT_TRUE(mgr->m_pendingErrors.contains("/dev/utblk"));
    EXPECT_EQ(mgr->m_pendingErrors.value("/dev/utblk").deviceName, QString("UT-Disk"));
    EXPECT_EQ(mgr->m_pendingErrors.value("/dev/utblk").fsType, QString("vfat"));
}

// onFsErrorDetected：只读硬件错误 → 普通通知（15s）
TEST_F(DockItemDataManagerTest, OnFsErrorDetected_ReadOnly_SendsPlainNotification)
{
    // Act
    mgr->onFsErrorDetected("/dev/utro", "RO-Disk", "vfat", "read_only", false, "msg");

    // Assert
    ASSERT_TRUE(ut::notifyMock().methods.contains("Notify"));
    const QVariantList args = ut::notifyMock().args.at(ut::notifyMock().methods.indexOf("Notify"));
    EXPECT_TRUE(args.value(5).toStringList().isEmpty());
    EXPECT_EQ(args.value(7).toInt(), 15000);
    EXPECT_TRUE(mgr->m_pendingErrors.contains("/dev/utro"));
}

// onFsErrorDetected：其它不可修复错误 → 严重损坏通知
TEST_F(DockItemDataManagerTest, OnFsErrorDetected_OtherFailure_SendsDamageNotification)
{
    // Act
    mgr->onFsErrorDetected("/dev/utunk", "UNK-Disk", "", "unknown_fs", false, "msg");

    // Assert
    ASSERT_TRUE(ut::notifyMock().methods.contains("Notify"));
    EXPECT_EQ(ut::notifyMock().args.last().value(7).toInt(), 15000);
    EXPECT_TRUE(mgr->m_pendingErrors.contains("/dev/utunk"));
}

// onFsErrorCleared：清理待处理错误 + 关联通知
TEST_F(DockItemDataManagerTest, OnFsErrorCleared_RemovesPendingAndClosesNotification)
{
    // Arrange
    mgr->m_pendingErrors["/dev/utblk"] = { "UT-Disk", "vfat", "dirty", "m" };
    mgr->m_notificationToDevice[77] = "/dev/utblk";

    // Act
    mgr->onFsErrorCleared("/dev/utblk");

    // Assert
    EXPECT_FALSE(mgr->m_pendingErrors.contains("/dev/utblk"));
    EXPECT_FALSE(mgr->m_notificationToDevice.contains(77));
    ASSERT_TRUE(ut::notifyMock().methods.contains("CloseNotification"));
    EXPECT_EQ(ut::notifyMock().args.last().value(0).toUInt(), 77U);

    // 无关联通知时不再发 CloseNotification
    ut::notifyMock().reset();
    mgr->onFsErrorCleared("/dev/none");
    EXPECT_FALSE(ut::notifyMock().methods.contains("CloseNotification"));
}

// notifyWithActions：应答完成后登记通知号（watcher lambda 成功分支）
TEST_F(DockItemDataManagerTest, NotifyWithActions_ReplyLambda_RecordsNotificationId)
{
    // Arrange
    ut::notifyMock().notifyId = 88;

    // Act
    mgr->notifyWithActions("t", "m", { "repair", "Fix" }, "/dev/utblk");
    QTest::qWait(200);

    // Assert：mock Notifications 返回 88，watcher lambda 登记映射
    EXPECT_TRUE(mgr->m_notificationToDevice.contains(88));
    EXPECT_EQ(mgr->m_notificationToDevice.value(88), QString("/dev/utblk"));
}

// onNotifyActionInvoked：未知通知号直接返回
TEST_F(DockItemDataManagerTest, OnNotifyActionInvoked_UnknownId_NothingHappens)
{
    // Arrange
    int detachedCount = 0;
    stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(
                           &QProcess::startDetached),
                   [&detachedCount](const QString &, const QStringList &, const QString &, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        ++detachedCount;
        return true;
    });

    // Act
    mgr->onNotifyActionInvoked(999, "repair");

    // Assert
    EXPECT_EQ(detachedCount, 0);
    EXPECT_FALSE(ut::notifyMock().methods.contains("CloseNotification"));
}

// onNotifyActionInvoked：repair 动作拉起独立修复对话框进程
TEST_F(DockItemDataManagerTest, OnNotifyActionInvoked_RepairAction_LaunchesDialogProcess)
{
    // Arrange
    mgr->m_pendingErrors["/dev/utblk"] = { "UT-Disk", "vfat", "dirty", "m" };
    mgr->m_notificationToDevice[88] = "/dev/utblk";
    mgr->blocks.insert("/dev/utblk", DockItemData { .id = "utblk", .totalSize = 2048ULL * 1024 * 1024 });

    QString program;
    QStringList progArgs;
    stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(
                           &QProcess::startDetached),
                   [&program, &progArgs](const QString &p, const QStringList &a, const QString &, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        program = p;
        progArgs = a;
        return true;
    });

    // Act
    mgr->onNotifyActionInvoked(88, "repair");

    // Assert
    EXPECT_EQ(program, QString("dde-usb-repair-dialog"));
    EXPECT_TRUE(progArgs.contains(QString("/dev/utblk")));
    EXPECT_TRUE(progArgs.contains(QString("UT-Disk")));
    EXPECT_TRUE(progArgs.contains(QString("vfat")));
    EXPECT_TRUE(ut::notifyMock().methods.contains("CloseNotification"));
    EXPECT_FALSE(mgr->m_notificationToDevice.contains(88));
    EXPECT_TRUE(mgr->m_pendingErrors.contains("/dev/utblk"));   // repair 保留待处理项
}

// onNotifyActionInvoked：ignore 动作仅清理
TEST_F(DockItemDataManagerTest, OnNotifyActionInvoked_IgnoreAction_CleansUpOnly)
{
    // Arrange
    mgr->m_pendingErrors["/dev/utign"] = { "IGN", "vfat", "dirty", "m" };
    mgr->m_notificationToDevice[66] = "/dev/utign";
    int detachedCount = 0;
    stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(
                           &QProcess::startDetached),
                   [&detachedCount](const QString &, const QStringList &, const QString &, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        ++detachedCount;
        return true;
    });

    // Act
    mgr->onNotifyActionInvoked(66, "ignore");

    // Assert
    EXPECT_EQ(detachedCount, 0);
    EXPECT_FALSE(mgr->m_pendingErrors.contains("/dev/utign"));
    EXPECT_FALSE(mgr->m_notificationToDevice.contains(66));
    EXPECT_TRUE(ut::notifyMock().methods.contains("CloseNotification"));
}

// onNotifyClosed：空实现，调用无副作用
TEST_F(DockItemDataManagerTest, OnNotifyClosed_KeepsMapping)
{
    // Arrange
    mgr->m_notificationToDevice[88] = "/dev/utblk";

    // Act
    mgr->onNotifyClosed(88, 2);

    // Assert：按设计显式保留映射
    EXPECT_TRUE(mgr->m_notificationToDevice.contains(88));
}

// onRepairProgress：更新已登记对话框的进度
TEST_F(DockItemDataManagerTest, OnRepairProgress_UpdatesDialogProgress)
{
    // Arrange
    RepairDialog dialog;
    dialog.setState(RepairDialog::kRepairing);   // 创建进度控件
    dialog.show();   // 父窗可见后进度控件才报告 isVisible
    QTest::qWait(20);
    mgr->m_repairDialogs["/dev/utblk"] = &dialog;

    // Act
    mgr->onRepairProgress("/dev/utblk", 42, "fsck 42%");

    // Assert
    ASSERT_NE(dialog.m_progressWidget, nullptr);
    EXPECT_EQ(dialog.m_progressWidget->value(), 42);
    EXPECT_TRUE(dialog.m_progressWidget->isVisible());

    // 无对话框的设备：安全无操作
    mgr->onRepairProgress("/dev/none", 10, "");
    EXPECT_EQ(dialog.getState(), RepairDialog::kRepairing);
}

// onRepairFinished：无对话框 → 清理待处理错误并早退
TEST_F(DockItemDataManagerTest, OnRepairFinished_NoDialog_CleansPendingAndReturns)
{
    // Arrange
    mgr->m_pendingErrors["/dev/utx"] = { "X", "vfat", "dirty", "m" };

    // Act
    mgr->onRepairFinished("/dev/utx", false, "io error");

    // Assert
    EXPECT_FALSE(mgr->m_pendingErrors.contains("/dev/utx"));
}

// onRepairFinished：成功且挂载成功 → 对话框置 kSuccess，按钮回调可打开设备
TEST_F(DockItemDataManagerTest, OnRepairFinished_SuccessMountsAndShowsSuccess)
{
    // Arrange：取 /proc/mounts 中真实存在的设备当 devicePath
    QString realDev;
    QFile mounts("/proc/mounts");
    if (mounts.open(QIODevice::ReadOnly)) {
        const QString firstLine = QString::fromUtf8(mounts.readLine());
        mounts.close();
        realDev = firstLine.section(' ', 0, 0);
    }
    ASSERT_FALSE(realDev.isEmpty());

    RepairDialog dialog;
    dialog.setDeviceInfo("UT-Disk", "2 GB", "VFAT");
    dialog.setState(RepairDialog::kConfirm);
    mgr->m_repairDialogs[realDev] = &dialog;
    mgr->m_pendingErrors[realDev] = { "UT-Disk", "vfat", "dirty", "m" };

    // 桩掉挂载子进程
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODeviceBase::OpenMode)>(
                           &QProcess::start),
                   [](QProcess *, const QString &, const QStringList &, QIODeviceBase::OpenMode) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(static_cast<int (QProcess::*)() const>(&QProcess::exitCode),
                   [](QProcess *) -> int { __DBG_STUB_INVOKE__ return 0; });

    // Act
    mgr->onRepairFinished(realDev, true, "");

    // Assert
    EXPECT_EQ(dialog.getState(), RepairDialog::kSuccess);
    EXPECT_FALSE(dialog.mountPoint().isEmpty());
    EXPECT_FALSE(mgr->m_pendingErrors.contains(realDev));

    // 按钮回调：Open Device → 拉起 dde-file-manager
    QString program;
    stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(
                           &QProcess::startDetached),
                   [&program](const QString &p, const QStringList &, const QString &, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        program = p;
        return true;
    });
    QMetaObject::invokeMethod(&dialog, "buttonClicked",
                              Q_ARG(int, 1), Q_ARG(QString, QString("Open Device")));
    EXPECT_EQ(program, QString("dde-file-manager"));
}

// onRepairFinished：挂载失败（设备不在 /proc/mounts）→ 视为失败
TEST_F(DockItemDataManagerTest, OnRepairFinished_MountFails_FallsBackToFailed)
{
    // Arrange
    RepairDialog dialog;
    dialog.setState(RepairDialog::kConfirm);
    mgr->m_repairDialogs["/dev/utnotmounted"] = &dialog;
    mgr->m_pendingErrors["/dev/utnotmounted"] = { "UT", "vfat", "dirty", "m" };

    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODeviceBase::OpenMode)>(
                           &QProcess::start),
                   [](QProcess *, const QString &, const QStringList &, QIODeviceBase::OpenMode) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(static_cast<int (QProcess::*)() const>(&QProcess::exitCode),
                   [](QProcess *) -> int { __DBG_STUB_INVOKE__ return 0; });

    // Act
    mgr->onRepairFinished("/dev/utnotmounted", true, "");

    // Assert
    EXPECT_EQ(dialog.getState(), RepairDialog::kFailed);
    EXPECT_EQ(dialog.m_errorCode, QString("Repair operation failed"));   // summary 空 → 默认错误码
}

// onRepairFinished：失败 + summary → 显示 summary 错误码
TEST_F(DockItemDataManagerTest, OnRepairFinished_FailureWithSummary_ShowsSummary)
{
    // Arrange
    RepairDialog dialog;
    dialog.setState(RepairDialog::kRepairing);
    mgr->m_repairDialogs["/dev/utfail"] = &dialog;
    mgr->m_pendingErrors["/dev/utfail"] = { "UT", "vfat", "dirty", "m" };

    // Act
    mgr->onRepairFinished("/dev/utfail", false, "cannot read superblock");

    // Assert
    EXPECT_EQ(dialog.getState(), RepairDialog::kFailed);
    EXPECT_EQ(dialog.m_errorCode, QString("cannot read superblock"));
    EXPECT_FALSE(mgr->m_pendingErrors.contains("/dev/utfail"));
}

// buildBlockItem / buildProtocolItem：字段映射与图标别名
TEST_F(DockItemDataManagerTest, BuildItems_FieldMappingAndIconAliases)
{
    // Arrange
    const QVariantMap blk = makeBlockData();

    // Act
    const DockItemData blkItem = mgr->buildBlockItem(blk);

    // Assert
    EXPECT_EQ(blkItem.id, QString("utblk"));
    EXPECT_EQ(blkItem.displayName, QString("UT-Disk"));
    EXPECT_EQ(blkItem.iconName, QString("drive-removable-media-usb"));
    EXPECT_EQ(blkItem.sortKey, QString("00drive-removable-media-usb_00UT-Disk"));
    EXPECT_FALSE(blkItem.isProtocolDevice);

    // 协议设备：mtp/gphoto → android-device；Apple/afc → ios-device
    // （protocolDeviceIcon 依赖主题是否含 "phone" 图标，桩掉保证确定性）
    stub.set_lamda(&device_utils::protocolDeviceIcon,
                   [](const QVariantMap &) -> QString { __DBG_STUB_INVOKE__ return "phone"; });
    QVariantMap proto = makeProtoData();
    proto[DeviceProperty::kId] = QString("gphoto2://[usb:1,2]");
    proto[DeviceProperty::kDeviceIcon] = QStringList { "phone" };
    DockItemData p1 = mgr->buildProtocolItem(proto);
    EXPECT_EQ(p1.iconName, QString("android-device"));
    EXPECT_TRUE(p1.sortKey.startsWith("01android-device"));
    EXPECT_TRUE(p1.isProtocolDevice);

    proto[DeviceProperty::kId] = QString("afc://Apple_Inc_iPhone");
    DockItemData p2 = mgr->buildProtocolItem(proto);
    EXPECT_EQ(p2.iconName, QString("ios-device"));

    // 普通协议设备保留原始图标名
    proto[DeviceProperty::kId] = QString("smb://nas/share");
    DockItemData p3 = mgr->buildProtocolItem(proto);
    EXPECT_EQ(p3.iconName, QString("phone"));   // 桩透传名
}

// closeNotification：发送 CloseNotification
TEST_F(DockItemDataManagerTest, CloseNotification_SendsDbusClose)
{
    // Act
    mgr->closeNotification(123);

    // Assert
    ASSERT_TRUE(ut::notifyMock().methods.contains("CloseNotification"));
    EXPECT_EQ(ut::notifyMock().args.last().value(0).toUInt(), 123U);
}
