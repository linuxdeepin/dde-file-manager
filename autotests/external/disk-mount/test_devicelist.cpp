// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_devicelist.cpp - DeviceList 单元测试
// 列表构造会触发 DockItemDataManager 单例的 initialize()，
// 为保证确定性走 test_mock_dbus_env 私有总线（见 test_mock_dbus_env.h）。

#include <gtest/gtest.h>

#include <QApplication>
#include <QSignalSpy>
#include <QtTest>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QVBoxLayout>

#include <DGuiApplicationHelper>

#include "stubext.h"

#include "test_mock_dbus_env.h"
#include "devicelist.h"
#include "dockitemdatamanager.h"
#include "deviceitem.h"
#include "global_server_defines.h"

using namespace GlobalServerDefines;
using namespace Dtk::Gui;

namespace {

DockItemData makeItemData(const QString &id, const QString &sortKey)
{
    return {
        .id = id,
        .backingID = id,
        .displayName = QString("dev-") + id,
        .targetUrl = QUrl::fromLocalFile("/media/" + id),
        .targetFileUrl = QUrl::fromLocalFile("/media/" + id),
        .iconName = QString("drive-removable-media-usb"),
        .totalSize = 1024ULL * 1024 * 1024,
        .usedSize = 256ULL * 1024 * 1024,
        .sortKey = sortKey,
        .isProtocolDevice = false,
    };
}

}   // namespace

class DeviceListTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        ASSERT_TRUE(ut::ensurePrivateBus()) << "私有 DBus 总线启动失败";
        ut::installSessionBusStub(stub);
        ut::devMngMock().reset();   // 默认空设备表，单例 initialize 结果确定
        ut::notifyMock().reset();
        qRegisterMetaType<DockItemData>("DockItemData");
    }

    void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

// 构造：initUI + initConnect 完成，头部与滚动区就位
TEST_F(DeviceListTest, Construct_BuildsHeaderAndScrollArea)
{
    // Arrange / Act
    DeviceList list;

    // Assert
    EXPECT_EQ(list.objectName(), QString("DiskControlWidget-QScrollArea"));
    EXPECT_EQ(list.width(), kDockPluginWidth);
    ASSERT_NE(list.headerWidget, nullptr);
    EXPECT_GE(list.headerWidget->sizeHint().height(), 0);
    EXPECT_TRUE(list.deviceItems.isEmpty());
    // 构造中经单例 initialize() 发起过设备列表查询
    EXPECT_TRUE(ut::devMngMock().methods.contains("GetBlockDevicesIdList"));
    EXPECT_NE(DockItemDataManager::instance(), nullptr);
}

// addDevice：新增条目入表并按 sortKey 排序插入
TEST_F(DeviceListTest, AddDevice_InsertsAndOrdersItems)
{
    // Arrange
    DeviceList list;
    list.addDevice(makeItemData("b2", "00z"));
    const int heightWithOne = list.height();

    // Act
    list.addDevice(makeItemData("b1", "00a"));

    // Assert
    ASSERT_EQ(list.deviceItems.count(), 2);
    EXPECT_TRUE(list.deviceItems.contains("b1"));
    ASSERT_EQ(list.deviceItems.count("b2"), 1);
    // 排序键记录与布局顺序：b1 的 sortKey 更小，应排在前面
    const int idxB1 = list.deviceLay->indexOf(list.deviceItems.value("b1"));
    const int idxB2 = list.deviceLay->indexOf(list.deviceItems.value("b2"));
    EXPECT_LT(idxB1, idxB2);
    EXPECT_GT(list.height(), heightWithOne);   // updateHeight 生效
}

// addDevice：重复 id 先移除再新增（条目数不涨）
TEST_F(DeviceListTest, AddDevice_DuplicateId_ReplacesExisting)
{
    // Arrange
    DeviceList list;
    list.addDevice(makeItemData("dup", "00a"));

    // Act
    list.addDevice(makeItemData("dup", "00b"));

    // Assert
    ASSERT_EQ(list.deviceItems.count(), 1);
    EXPECT_EQ(list.sortKeys.value("dup"), QString("00b"));
}

// removeDevice：未知 id 安全返回；已知 id 移除并收缩高度
TEST_F(DeviceListTest, RemoveDevice_KnownAndUnknown_UpdatesHeight)
{
    // Arrange
    DeviceList list;
    list.addDevice(makeItemData("r1", "00a"));
    const int heightWithItem = list.height();

    // Act：未知 id
    list.removeDevice("no-such");
    EXPECT_EQ(list.deviceItems.count(), 1);

    // Act：已知 id
    list.removeDevice("r1");

    // Assert
    EXPECT_TRUE(list.deviceItems.isEmpty());
    EXPECT_TRUE(list.sortKeys.isEmpty());
    EXPECT_LT(list.height(), heightWithItem);
}

// ejectDevice：分发到单例 → DBus Detach 调用
TEST_F(DeviceListTest, EjectDevice_DispatchesToManager)
{
    // Arrange
    DeviceList list;

    // Act
    list.ejectDevice("/org/freedesktop/UDisks2/block_devices/utb");

    // Assert
    EXPECT_TRUE(ut::devMngMock().methods.contains("DetachBlockDevice"));
}

// showEvent / hideEvent：可见性切换驱动容量订阅
TEST_F(DeviceListTest, ShowAndHide_TriggersUsageMonitoringCalls)
{
    // Arrange
    DeviceList list;

    // Act
    list.show();
    QTest::qWait(100);
    const QStringList afterShow = ut::devMngMock().methods;
    list.hide();
    QTest::qWait(100);
    const QStringList afterHide = ut::devMngMock().methods;

    // Assert：显示时 refresh + subscribe，隐藏时 unsubscribe
    EXPECT_TRUE(afterShow.contains("RefreshDeviceUsage"));
    EXPECT_TRUE(afterShow.contains("StartMonitoringUsage"));
    EXPECT_TRUE(afterHide.contains("StopMonitoringUsage"));
}

// usageUpdated lambda：容量更新路由到对应条目
TEST_F(DeviceListTest, UsageUpdated_Lambda_UpdatesCorrespondingItem)
{
    // Arrange
    DeviceList list;
    list.addDevice(makeItemData("u1", "00a"));
    auto *item = dynamic_cast<DeviceItem *>(list.deviceItems.value("u1"));
    ASSERT_NE(item, nullptr);
    QLabel *sizeLabel = item->findChildren<QLabel *>().last();   // sizeLabel 后创建
    const QString before = sizeLabel->text();

    // Act：直接触发单例信号（与 DBus 路径等价的 lambda 连线）
    Q_EMIT DockItemDataManager::instance()->usageUpdated("u1", 768ULL * 1024 * 1024);
    QTest::qWait(20);

    // Assert
    EXPECT_NE(sizeLabel->text(), before);
    EXPECT_TRUE(sizeLabel->text().contains("768"));
    // 不存在的设备 id 安全忽略
    Q_EMIT DockItemDataManager::instance()->usageUpdated("ghost", 1);
    QTest::qWait(20);
    SUCCEED() << "unknown id ignored";
}

// createHeader 主题 lambda：主题切换不崩溃且头部仍在
TEST_F(DeviceListTest, CreateHeader_ThemeChangeLambda_Safe)
{
    // Arrange
    DeviceList list;
    ASSERT_NE(list.headerWidget, nullptr);

    // Act：dtk6 无 setThemeType，直接触发主题变更信号
    Q_EMIT DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::DarkType);
    QTest::qWait(20);
    Q_EMIT DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::LightType);   // 还原

    // Assert
    EXPECT_NE(list.headerWidget->sizeHint().height(), 0);
    EXPECT_FALSE(list.grab().isNull());   // 离屏渲染触发 paintEvent
}
