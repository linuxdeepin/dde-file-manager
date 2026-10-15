// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_diskmountplugin.cpp - DiskMountPlugin（dde-dock 插件接口）单元测试
// 伪 PluginProxyInterface 记录交互；loadTranslator 为 DApplication 非虚方法，
// 测试进程是 QApplication，直接打桩保证安全；DBus 走 test_mock_dbus_env 私有总线。

#include <gtest/gtest.h>

#include <QApplication>
#include <QSignalSpy>
#include <QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>

#include <DApplication>
#include <DConfig>

#include "stubext.h"

#include "test_mock_dbus_env.h"
#include "diskmountplugin.h"
#include "widgets/tipswidget.h"
#include "widgets/diskpluginitem.h"
#include "widgets/devicelist.h"
#include "device/dockitemdatamanager.h"
#include "utils/dockutils.h"
#include "global_server_defines.h"

using namespace GlobalServerDefines;
using namespace Dtk::Widget;
using namespace Dtk::Core;   // DConfig

namespace {

// dde-dock 代理接口桩：记录插件与任务栏的交互
class FakeProxyInter : public PluginProxyInterface
{
public:
    void itemAdded(PluginsItemInterface *const, const QString &key) override { addedKeys << key; }
    void itemUpdate(PluginsItemInterface *const, const QString &key) override { updatedKeys << key; }
    void itemRemoved(PluginsItemInterface *const, const QString &key) override { removedKeys << key; }
    void requestWindowAutoHide(PluginsItemInterface *const, const QString &, const bool) override {}
    void requestRefreshWindowVisible(PluginsItemInterface *const, const QString &) override {}
    void requestSetAppletVisible(PluginsItemInterface *const, const QString &, const bool) override {}
    void saveValue(PluginsItemInterface *const, const QString &key, const QVariant &value) override
    {
        saved[key] = value;
    }
    const QVariant getValue(PluginsItemInterface *const, const QString &key,
                            const QVariant &fallback = QVariant()) override
    {
        return values.contains(key) ? values.value(key) : fallback;
    }
    void removeValue(PluginsItemInterface *const, const QStringList &) override {}

    QStringList addedKeys;
    QStringList updatedKeys;
    QStringList removedKeys;
    QMap<QString, QVariant> saved;
    QMap<QString, QVariant> values;
};

}   // namespace

class DiskMountPluginTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        ASSERT_TRUE(ut::ensurePrivateBus()) << "私有 DBus 总线启动失败";
        ut::installSessionBusStub(stub);
        ut::devMngMock().reset();   // 默认空设备表，单例 initialize 结果确定
        ut::notifyMock().reset();
        // 测试进程为 QApplication：qApp 被 DApplication 头静态转型，loadTranslator
        // 为非虚方法可安全打桩（同时避免真实翻译器加载）。
        stub.set_lamda(static_cast<bool (DApplication::*)(QList<QLocale>)>(&DApplication::loadTranslator),
                       [](DApplication *, QList<QLocale>) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
    }

    void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
    FakeProxyInter proxy;
};

// 声明顺序即执行顺序：init 的 call_once 逻辑依赖本文件首测先行
// init：注册代理 → 构建组件 → 隐藏初始条目
TEST_F(DiskMountPluginTest, Init_WithProxy_RegistersAndBuildsComponents)
{
    // Arrange
    DiskMountPlugin plugin;

    // Act
    plugin.init(&proxy);

    // Assert
    EXPECT_EQ(plugin.proxyInter(), &proxy);
    EXPECT_EQ(plugin.pluginName(), QString("disk-mount"));
    EXPECT_EQ(plugin.flags(), Dock::Type_Tray | Dock::Attribute_CanDrag | Dock::Attribute_CanInsert);
    EXPECT_NE(plugin.itemWidget("mount-item-key"), nullptr);
    EXPECT_NE(plugin.itemTipsWidget("mount-item-key"), nullptr);
    EXPECT_NE(plugin.itemPopupApplet("mount-item-key"), nullptr);   // DeviceList 已创建
    EXPECT_FALSE(plugin.tipsLabel->isVisible());
    EXPECT_FALSE(plugin.diskControlApplet->isVisible());
}

// 再次 init：once_flag 已消耗 → 新实例的 init 不再执行初始化体
TEST_F(DiskMountPluginTest, Init_SecondCall_IsGuardedByOnceFlag)
{
    // Arrange（Init_WithProxy 已消耗 once_flag，本用例声明序在其后）
    DiskMountPlugin plugin;
    FakeProxyInter secondProxy;

    // Act：新实例调用 init 传入不同代理
    plugin.init(&secondProxy);

    // Assert：初始化体被跳过，成员代理指针未被赋值
    EXPECT_EQ(plugin.proxyInter(), nullptr);
    EXPECT_NE(plugin.proxyInter(), &secondProxy);
}

// itemContextMenu：DConfig 失效 → 默认集成 → 菜单含 open 与 eject_all
TEST_F(DiskMountPluginTest, ItemContextMenu_DefaultIntegration_ContainsOpenAndEjectAll)
{
    // Arrange
    DiskMountPlugin plugin;
    stub.set_lamda(static_cast<DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(
                           &DConfig::create),
                   [](const QString &, const QString &, const QString &, QObject *) -> DConfig * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Act
    const QString json = plugin.itemContextMenu("mount-item-key");

    // Assert
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    ASSERT_TRUE(doc.isObject());
    const QJsonArray items = doc.object().value("items").toArray();
    ASSERT_EQ(items.count(), 2);
    EXPECT_EQ(items.at(0).toObject().value("itemId").toString(), QString("open"));
    EXPECT_EQ(items.at(1).toObject().value("itemId").toString(), QString("eject_all"));
    EXPECT_EQ(doc.object().value("checkableMenu").toBool(), false);
}

// invokedMenuItem：open → gio 打开 computer:///；未知项无动作
TEST_F(DiskMountPluginTest, InvokedMenuItem_Open_InvokesGioUnknownIgnored)
{
    // Arrange
    DiskMountPlugin plugin;
    plugin.init(&proxy);
    QString program;
    QStringList args;
    stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(&QProcess::startDetached),
                   [&program, &args](const QString &p, const QStringList &a, const QString &, qint64 *) -> bool {
        __DBG_STUB_INVOKE__
        program = p;
        args = a;
        return true;
    });

    // Act：open
    plugin.invokedMenuItem("mount-item-key", "open", false);

    // Assert
    const QStringList expectedArgs { "open", "computer:///" };
    EXPECT_EQ(program, QString("gio"));
    EXPECT_EQ(args, expectedArgs);

    // Act：未知菜单项 → 无进程
    program.clear();
    plugin.invokedMenuItem("mount-item-key", "no-such-item", false);
    EXPECT_TRUE(program.isEmpty());
}

// invokedMenuItem：eject_all → 单例弹出全部设备
TEST_F(DiskMountPluginTest, InvokedMenuItem_EjectAll_CallsDetachAll)
{
    // Arrange
    DiskMountPlugin plugin;
    plugin.init(&proxy);   // once_flag 已消耗，此处等价于 no-op

    // Act
    plugin.invokedMenuItem("mount-item-key", "eject_all", false);

    // Assert
    EXPECT_TRUE(ut::devMngMock().methods.contains("DetachAllMountedDevices"));
}

// itemSortKey / setSortKey：经代理存取排序键
TEST_F(DiskMountPluginTest, SortKey_GetAndSet_GoThroughProxy)
{
    // Arrange：once_flag 已消耗，直接设置代理指针
    DiskMountPlugin plugin;
    plugin.setProxyInter(&proxy);
    proxy.values.insert("pos_mount-item-key_1", 7);

    // Act / Assert：读
    EXPECT_EQ(plugin.itemSortKey("mount-item-key"), 7);
    EXPECT_EQ(plugin.itemSortKey("any-key"), 0);   // 默认回退 0

    // Act / Assert：写
    plugin.setSortKey("mount-item-key", 9);
    EXPECT_EQ(proxy.saved.value("pos_mount-item-key_1").toInt(), 9);
}

// setDockEntryVisible：向任务栏加/删条目并同步图标可见性
TEST_F(DiskMountPluginTest, SetDockEntryVisible_TogglesItemAndWidget)
{
    // Arrange：once_flag 已消耗，直接设置代理指针
    DiskMountPlugin plugin;
    plugin.setProxyInter(&proxy);

    // Act：显示
    plugin.setDockEntryVisible(true);

    // Assert
    ASSERT_EQ(proxy.addedKeys.count(), 1);
    EXPECT_EQ(proxy.addedKeys.first(), QString("mount-item-key"));
    EXPECT_TRUE(plugin.diskPluginItem->isVisible());

    // Act：隐藏
    plugin.setDockEntryVisible(false);

    // Assert
    ASSERT_EQ(proxy.removedKeys.count(), 1);
    EXPECT_EQ(proxy.removedKeys.first(), QString("mount-item-key"));
    EXPECT_FALSE(plugin.diskPluginItem->isVisible());
}

// refreshIcon：匹配键 → 刷新图标；其它键忽略
TEST_F(DiskMountPluginTest, RefreshIcon_MatchingKey_UpdatesIcon)
{
    // Arrange
    DiskMountPlugin plugin;
    plugin.init(&proxy);

    // Act / Assert
    plugin.refreshIcon("mount-item-key");
    EXPECT_FALSE(plugin.diskPluginItem->getIcon().isNull());

    plugin.refreshIcon("other-key");   // 忽略分支
    EXPECT_FALSE(plugin.diskPluginItem->getIcon().isNull());
}

// displayModeChanged：透传给图标项
TEST_F(DiskMountPluginTest, DisplayModeChanged_ForwardedToItem)
{
    // Arrange
    DiskMountPlugin plugin;
    plugin.init(&proxy);

    // Act
    plugin.displayModeChanged(Dock::Fashion);

    // Assert
    EXPECT_EQ(plugin.diskPluginItem->displayMode, Dock::Fashion);
}

// setProxyInter：直接设置/取回
TEST_F(DiskMountPluginTest, SetProxyInter_DirectSetter_UpdatesGetter)
{
    // Arrange
    DiskMountPlugin plugin;

    // Act
    plugin.setProxyInter(&proxy);

    // Assert
    EXPECT_EQ(plugin.proxyInter(), &proxy);
}

// onceFlag：静态单次标记可重复取用
TEST_F(DiskMountPluginTest, OnceFlag_StaticReference_Stable)
{
    // Arrange / Act
    std::once_flag &flag1 = DiskMountPlugin::onceFlag();
    std::once_flag &flag2 = DiskMountPlugin::onceFlag();

    // Assert
    EXPECT_EQ(&flag1, &flag2);
}

// isIntegratedByFilemanager（dockutils 补充覆盖）：DConfig 创建失败 → 默认集成
TEST_F(DiskMountPluginTest, IsIntegratedByFilemanager_NoDConfig_DefaultsTrue)
{
    // Arrange
    stub.set_lamda(static_cast<DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(
                           &DConfig::create),
                   [](const QString &, const QString &, const QString &, QObject *) -> DConfig * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Act
    const bool integrated = common_utils::isIntegratedByFilemanager();

    // Assert
    EXPECT_TRUE(integrated);
}
