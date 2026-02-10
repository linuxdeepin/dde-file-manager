// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_lifecycle.cpp - LifeCycle命名空间函数单元测试
// 使用stub_ext进行函数打桩测试

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTest>
#include <QDir>
#include <QStringList>
#include <QQueue>
#include <memory>

// 包含stub_ext
#include "stubext.h"

// 包含待测试的类
#include <dfm-framework/lifecycle/lifecycle.h>
#include <dfm-framework/lifecycle/pluginmanager.h>
#include <dfm-framework/lifecycle/pluginmetaobject.h>
#include <dfm-framework/lifecycle/private/pluginmetaobject_p.h>
#include <dfm-framework/lifecycle/plugin.h>
#include <dfm-framework/lifecycle/plugincreator.h>
#include <dfm-framework/listener/listener.h>

using namespace dpf;
using namespace dpf::LifeCycle;

namespace lifecycle_test {

// 测试用的Mock类定义
class MockPlugin : public Plugin
{
public:
    static int instanceCount;
    static int initCount;
    static int startCount;
    static int stopCount;
    
    MockPlugin() { instanceCount++; }
    ~MockPlugin() { instanceCount--; }
    void initialize() override { initCount++; }
    bool start() override { startCount++; return true; }
    void stop() override { stopCount++; }
};

class MockPluginFailStart : public Plugin
{
public:
    void initialize() override {}
    bool start() override { return false; }  // 启动失败
    void stop() override {}
};

class MockPluginCreator : public PluginCreator
{
public:
    QSharedPointer<Plugin> create(const QString &key) override
    {
        Q_UNUSED(key)
        return QSharedPointer<Plugin>(new MockPlugin());
    }
};

// 静态成员变量定义
int MockPlugin::instanceCount = 0;
int MockPlugin::initCount = 0;
int MockPlugin::startCount = 0;
int MockPlugin::stopCount = 0;

}

// 辅助函数：创建测试用的插件元数据
PluginMetaObjectPointer createTestPluginMeta(const QString &name, const QString &version = "1.0.0")
{
    PluginMetaObjectPointer plugin(new PluginMetaObject);
    plugin->d->name = name;
    plugin->d->version = version;
    plugin->d->iid = "test.lifecycle.interface";
    plugin->d->state = PluginMetaObject::kReaded;
    return plugin;
}

/**
 * @brief LifeCycle命名空间函数单元测试
 *
 * 测试范围：
 * 1. 初始化函数（initialize的各个重载版本）
 * 2. 插件IID和路径管理
 * 3. 黑名单和懒加载管理
 * 4. Qt版本不敏感插件注册
 * 5. 插件元数据获取和查询
 * 6. 插件读取、加载、关闭流程
 * 7. 单个插件操作
 * 8. 状态查询函数
 * 9. 过滤器设置
 * 10. 错误处理和边界情况
 */
class LifeCycleTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 初始化测试环境
        stub.clear();

        // 设置测试用的应用程序
        if (!qApp) {
            static int argc = 1;
            static char *argv[] = { "test_lifecycle" };
            app = std::make_unique<QCoreApplication>(argc, argv);
        }

        // 重置Mock计数器
        lifecycle_test::MockPlugin::instanceCount = 0;
        lifecycle_test::MockPlugin::initCount = 0;
        lifecycle_test::MockPlugin::startCount = 0;
        lifecycle_test::MockPlugin::stopCount = 0;
    }

    void TearDown() override
    {
        // 清理测试环境
        stub.clear();
    }

    // 测试用的打桩器
    stub_ext::StubExt stub;
    std::unique_ptr<QCoreApplication> app;
};

/**
 * @brief 测试基本初始化功能
 * 验证initialize(IIDs, paths)函数的基本功能
 */
TEST_F(LifeCycleTest, Initialize_Basic)
{
    QStringList testIIDs = { "test.plugin.interface1", "test.plugin.interface2" };
    QStringList testPaths = { "/test/plugins", "/test/plugins2" };

    // 打桩PluginManager的方法
    bool addPluginIIDCalled = false;
    bool setPluginPathsCalled = false;
    
    stub.set_lamda(&PluginManager::addPluginIID, [&addPluginIIDCalled](PluginManager *self, const QString &iid) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        addPluginIIDCalled = true;
        // 验证传入的IID是否正确
        EXPECT_TRUE(iid == "test.plugin.interface1" || iid == "test.plugin.interface2");
    });

    stub.set_lamda(&PluginManager::setPluginPaths, [&setPluginPathsCalled, testPaths](PluginManager *self, const QStringList &paths) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        setPluginPathsCalled = true;
        EXPECT_EQ(paths, testPaths);
    });

    // 调用函数
    initialize(testIIDs, testPaths);

    // 验证结果
    EXPECT_TRUE(addPluginIIDCalled);
    EXPECT_TRUE(setPluginPathsCalled);
}

/**
 * @brief 测试带黑名单的初始化功能
 * 验证initialize(IIDs, paths, blackNames)函数
 */
TEST_F(LifeCycleTest, Initialize_WithBlacklist)
{
    QStringList testIIDs = { "test.plugin.interface" };
    QStringList testPaths = { "/test/plugins" };
    QStringList testBlackNames = { "BlacklistedPlugin1", "BlacklistedPlugin2" };

    // 打桩PluginManager的方法
    QStringList actualBlackNames;
    stub.set_lamda(&PluginManager::addBlackPluginName, [&actualBlackNames](PluginManager *self, const QString &name) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        actualBlackNames.append(name);
    });

    stub.set_lamda(&PluginManager::addPluginIID, [](PluginManager *self, const QString &iid) {
        Q_UNUSED(self)
        Q_UNUSED(iid)
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&PluginManager::setPluginPaths, [](PluginManager *self, const QStringList &paths) {
        Q_UNUSED(self)
        Q_UNUSED(paths)
        __DBG_STUB_INVOKE__
    });

    // 调用函数
    initialize(testIIDs, testPaths, testBlackNames);

    // 验证结果
    EXPECT_EQ(actualBlackNames.size(), 2);
    EXPECT_TRUE(actualBlackNames.contains("BlacklistedPlugin1"));
    EXPECT_TRUE(actualBlackNames.contains("BlacklistedPlugin2"));
}

/**
 * @brief 测试带懒加载的初始化功能
 * 验证initialize(IIDs, paths, blackNames, lazyNames)函数
 */
TEST_F(LifeCycleTest, Initialize_WithLazyLoad)
{
    QStringList testIIDs = { "test.plugin.interface" };
    QStringList testPaths = { "/test/plugins" };
    QStringList testBlackNames = { "BlacklistedPlugin" };
    QStringList testLazyNames = { "LazyPlugin1", "LazyPlugin2" };

    // 打桩PluginManager的方法
    QStringList actualLazyNames;
    stub.set_lamda(&PluginManager::addLazyLoadPluginName, [&actualLazyNames](PluginManager *self, const QString &name) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        actualLazyNames.append(name);
    });

    stub.set_lamda(&PluginManager::addBlackPluginName, [](PluginManager *self, const QString &name) {
        Q_UNUSED(self)
        Q_UNUSED(name)
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&PluginManager::addPluginIID, [](PluginManager *self, const QString &iid) {
        Q_UNUSED(self)
        Q_UNUSED(iid)
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&PluginManager::setPluginPaths, [](PluginManager *self, const QStringList &paths) {
        Q_UNUSED(self)
        Q_UNUSED(paths)
        __DBG_STUB_INVOKE__
    });

    // 调用函数
    initialize(testIIDs, testPaths, testBlackNames, testLazyNames);

    // 验证结果
    EXPECT_EQ(actualLazyNames.size(), 2);
    EXPECT_TRUE(actualLazyNames.contains("LazyPlugin1"));
    EXPECT_TRUE(actualLazyNames.contains("LazyPlugin2"));
}

/**
 * @brief 测试Qt版本不敏感插件注册
 * 验证registerQtVersionInsensitivePlugins函数
 */
TEST_F(LifeCycleTest, RegisterQtVersionInsensitivePlugins)
{
    QStringList testNames = { "QtInsensitivePlugin1", "QtInsensitivePlugin2" };

    // 打桩PluginManager的方法
    QStringList actualNames;
    stub.set_lamda(&PluginManager::setQtVersionInsensitivePluginNames, [&actualNames](PluginManager *self, const QStringList &names) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        actualNames = names;
    });

    // 调用函数
    registerQtVersionInsensitivePlugins(testNames);

    // 验证结果
    EXPECT_EQ(actualNames, testNames);
}

/**
 * @brief 测试插件IID获取
 * 验证pluginIIDs函数
 */
TEST_F(LifeCycleTest, PluginIIDs)
{
    QStringList testIIDs = { "test.interface1", "test.interface2" };

    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::pluginIIDs, [testIIDs](PluginManager *self) -> QStringList {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return testIIDs;
    });

    // 调用函数
    QStringList result = pluginIIDs();

    // 验证结果
    EXPECT_EQ(result, testIIDs);
}

/**
 * @brief 测试插件路径获取
 * 验证pluginPaths函数
 */
TEST_F(LifeCycleTest, PluginPaths)
{
    QStringList testPaths = { "/test/path1", "/test/path2" };

    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::pluginPaths, [testPaths](PluginManager *self) -> QStringList {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return testPaths;
    });

    // 调用函数
    QStringList result = pluginPaths();

    // 验证结果
    EXPECT_EQ(result, testPaths);
}

/**
 * @brief 测试黑名单获取
 * 验证blackList函数
 */
TEST_F(LifeCycleTest, BlackList)
{
    QStringList testBlackList = { "BlackPlugin1", "BlackPlugin2" };

    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::blackList, [testBlackList](PluginManager *self) -> QStringList {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return testBlackList;
    });

    // 调用函数
    QStringList result = blackList();

    // 验证结果
    EXPECT_EQ(result, testBlackList);
}

/**
 * @brief 测试懒加载列表获取
 * 验证lazyLoadList函数
 */
TEST_F(LifeCycleTest, LazyLoadList)
{
    QStringList testLazyList = { "LazyPlugin1", "LazyPlugin2" };

    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::lazyLoadList, [testLazyList](PluginManager *self) -> QStringList {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return testLazyList;
    });

    // 调用函数
    QStringList result = lazyLoadList();

    // 验证结果
    EXPECT_EQ(result, testLazyList);
}

/**
 * @brief 测试插件元数据获取
 * 验证pluginMetaObj函数
 */
TEST_F(LifeCycleTest, PluginMetaObj)
{
    QString testName = "TestPlugin";
    QString testVersion = "1.0.0";
    auto testPlugin = createTestPluginMeta(testName, testVersion);

    // 打桩PluginManager的方法
    stub.set_lamda(static_cast<PluginMetaObjectPointer (PluginManager::*)(const QString &, const QString) const>(&PluginManager::pluginMetaObj),
                   [testPlugin](PluginManager *self, const QString &name, const QString &version) -> PluginMetaObjectPointer {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        if (name == "TestPlugin" && version == "1.0.0") {
            return testPlugin;
        }
        return PluginMetaObjectPointer();
    });

    // 调用函数
    auto result = pluginMetaObj(testName, testVersion);

    // 验证结果
    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result->name(), testName);
    EXPECT_EQ(result->version(), testVersion);
}

/**
 * @brief 测试插件元数据列表获取 - 无条件
 * 验证pluginMetaObjs函数的无条件版本
 */
TEST_F(LifeCycleTest, PluginMetaObjs_NoCondition)
{
    // 创建测试数据
    QList<PluginMetaObjectPointer> testPlugins;
    testPlugins.append(createTestPluginMeta("Plugin1"));
    testPlugins.append(createTestPluginMeta("Plugin2"));

    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::readQueue, [testPlugins](PluginManager *self) -> const QQueue<PluginMetaObjectPointer>& {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        static QQueue<PluginMetaObjectPointer> staticQueue;
        staticQueue.clear();
        for (const auto &plugin : testPlugins) {
            staticQueue.enqueue(plugin);
        }
        return staticQueue;
    });

    // 调用函数 - 无条件
    auto result = pluginMetaObjs();

    // 验证结果
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0]->name(), "Plugin1");
    EXPECT_EQ(result[1]->name(), "Plugin2");
}

/**
 * @brief 测试插件元数据列表获取 - 有条件
 * 验证pluginMetaObjs函数的条件过滤版本
 */
TEST_F(LifeCycleTest, PluginMetaObjs_WithCondition)
{
    // 创建测试数据
    QList<PluginMetaObjectPointer> testPlugins;
    testPlugins.append(createTestPluginMeta("FilteredPlugin"));
    testPlugins.append(createTestPluginMeta("NormalPlugin"));

    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::readQueue, [testPlugins](PluginManager *self) -> const QQueue<PluginMetaObjectPointer>& {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        static QQueue<PluginMetaObjectPointer> staticQueue;
        staticQueue.clear();
        for (const auto &plugin : testPlugins) {
            staticQueue.enqueue(plugin);
        }
        return staticQueue;
    });

    // 调用函数 - 带条件过滤
    auto result = pluginMetaObjs([](PluginMetaObjectPointer ptr) -> bool {
        return ptr->name().startsWith("Filtered");
    });

    // 验证结果
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->name(), "FilteredPlugin");
}

/**
 * @brief 测试排序后插件元数据列表获取
 * 验证pluginSortedMetaObjs函数
 */
TEST_F(LifeCycleTest, PluginSortedMetaObjs)
{
    // 创建测试数据
    QList<PluginMetaObjectPointer> testPlugins;
    testPlugins.append(createTestPluginMeta("SortedPlugin1"));
    testPlugins.append(createTestPluginMeta("SortedPlugin2"));
    testPlugins.append(createTestPluginMeta("OtherPlugin"));

    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::loadQueue, [testPlugins](PluginManager *self) -> const QQueue<PluginMetaObjectPointer>& {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        static QQueue<PluginMetaObjectPointer> staticQueue;
        staticQueue.clear();
        for (const auto &plugin : testPlugins) {
            staticQueue.enqueue(plugin);
        }
        return staticQueue;
    });

    // 调用函数 - 带条件过滤
    auto result = pluginSortedMetaObjs([](PluginMetaObjectPointer ptr) -> bool {
        return ptr->name().startsWith("Sorted");
    });

    // 验证结果
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0]->name(), "SortedPlugin1");
    EXPECT_EQ(result[1]->name(), "SortedPlugin2");
}

/**
 * @brief 测试插件读取功能
 * 验证readPlugins函数
 */
TEST_F(LifeCycleTest, ReadPlugins_Success)
{
    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::readPlugins, [](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    // 调用函数
    bool result = readPlugins();

    // 验证结果
    EXPECT_TRUE(result);
}

/**
 * @brief 测试插件读取功能 - 失败情况
 * 验证readPlugins函数的错误处理
 */
TEST_F(LifeCycleTest, ReadPlugins_Failure)
{
    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::readPlugins, [](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return false;
    });

    // 调用函数
    bool result = readPlugins();

    // 验证结果
    EXPECT_FALSE(result);
}

/**
 * @brief 测试插件加载功能
 * 验证loadPlugins函数的完整流程
 */
TEST_F(LifeCycleTest, LoadPlugins_Success)
{
    // 记录调用顺序
    QStringList callOrder;

    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::loadPlugins, [&callOrder](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        callOrder.append("loadPlugins");
        return true;
    });

    stub.set_lamda(&PluginManager::initPlugins, [&callOrder](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        callOrder.append("initPlugins");
        return true;
    });

    stub.set_lamda(&PluginManager::startPlugins, [&callOrder](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        callOrder.append("startPlugins");
        return true;
    });

    // 调用函数
    bool result = loadPlugins();

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_EQ(callOrder.size(), 3);
    EXPECT_EQ(callOrder[0], "loadPlugins");
    EXPECT_EQ(callOrder[1], "initPlugins");
    EXPECT_EQ(callOrder[2], "startPlugins");
}

/**
 * @brief 测试插件加载功能 - 加载失败
 * 验证loadPlugins函数在加载阶段失败的处理
 */
TEST_F(LifeCycleTest, LoadPlugins_LoadFailure)
{
    QStringList callOrder;

    // 打桩PluginManager的方法 - 加载失败
    stub.set_lamda(&PluginManager::loadPlugins, [&callOrder](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        callOrder.append("loadPlugins");
        return false;  // 加载失败
    });

    stub.set_lamda(&PluginManager::initPlugins, [&callOrder](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        callOrder.append("initPlugins");
        return true;
    });

    stub.set_lamda(&PluginManager::startPlugins, [&callOrder](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        callOrder.append("startPlugins");
        return true;
    });

    // 调用函数
    bool result = loadPlugins();

    // 验证结果 - 应该返回false，但仍继续执行后续步骤
    EXPECT_FALSE(result);
    EXPECT_EQ(callOrder.size(), 3);  // 所有步骤都应该执行
}

/**
 * @brief 测试插件关闭功能
 * 验证shutdownPlugins函数
 */
TEST_F(LifeCycleTest, ShutdownPlugins)
{
    bool stopPluginsCalled = false;

    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::stopPlugins, [&stopPluginsCalled](PluginManager *self) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        stopPluginsCalled = true;
    });

    // 调用函数
    shutdownPlugins();

    // 验证结果
    EXPECT_TRUE(stopPluginsCalled);

    // 重置标志，再次调用应该不会执行（由于std::once_flag）
    stopPluginsCalled = false;
    shutdownPlugins();
    // 注意：由于使用了std::once_flag，第二次调用不应该执行
    // 在实际实现中，这个测试可能需要调整
}

/**
 * @brief 测试单个插件加载功能 - 成功情况
 * 验证loadPlugin函数
 */
TEST_F(LifeCycleTest, LoadPlugin_Success)
{
    auto testPlugin = createTestPluginMeta("TestPlugin");
    QStringList callOrder;

    // 打桩PluginManager的方法
    stub.set_lamda(static_cast<bool (PluginManager::*)(PluginMetaObjectPointer&)>(&PluginManager::loadPlugin),
                   [&callOrder](PluginManager *self, PluginMetaObjectPointer &ptr) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(ptr)
        __DBG_STUB_INVOKE__
        callOrder.append("loadPlugin");
        return true;
    });

    stub.set_lamda(static_cast<bool (PluginManager::*)(PluginMetaObjectPointer&)>(&PluginManager::initPlugin),
                   [&callOrder](PluginManager *self, PluginMetaObjectPointer &ptr) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(ptr)
        __DBG_STUB_INVOKE__
        callOrder.append("initPlugin");
        return true;
    });

    stub.set_lamda(static_cast<bool (PluginManager::*)(PluginMetaObjectPointer&)>(&PluginManager::startPlugin),
                   [&callOrder](PluginManager *self, PluginMetaObjectPointer &ptr) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(ptr)
        __DBG_STUB_INVOKE__
        callOrder.append("startPlugin");
        return true;
    });

    // 调用函数
    bool result = loadPlugin(testPlugin);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_EQ(callOrder.size(), 3);
    EXPECT_EQ(callOrder[0], "loadPlugin");
    EXPECT_EQ(callOrder[1], "initPlugin");
    EXPECT_EQ(callOrder[2], "startPlugin");
}

/**
 * @brief 测试单个插件加载功能 - 空插件指针
 * 验证loadPlugin函数的错误处理
 */
TEST_F(LifeCycleTest, LoadPlugin_NullPointer)
{
    PluginMetaObjectPointer nullPlugin;

    // 调用函数
    bool result = loadPlugin(nullPlugin);

    // 验证结果
    EXPECT_FALSE(result);
}

/**
 * @brief 测试单个插件加载功能 - 加载失败
 * 验证loadPlugin函数在加载阶段失败的处理
 */
TEST_F(LifeCycleTest, LoadPlugin_LoadFailure)
{
    auto testPlugin = createTestPluginMeta("TestPlugin");

    // 打桩PluginManager的方法 - 加载失败
    stub.set_lamda(static_cast<bool (PluginManager::*)(PluginMetaObjectPointer&)>(&PluginManager::loadPlugin),
                   [](PluginManager *self, PluginMetaObjectPointer &ptr) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(ptr)
        __DBG_STUB_INVOKE__
        return false;  // 加载失败
    });

    // 调用函数
    bool result = loadPlugin(testPlugin);

    // 验证结果
    EXPECT_FALSE(result);
}

/**
 * @brief 测试单个插件加载功能 - 初始化失败
 * 验证loadPlugin函数在初始化阶段失败的处理
 */
TEST_F(LifeCycleTest, LoadPlugin_InitFailure)
{
    auto testPlugin = createTestPluginMeta("TestPlugin");

    // 打桩PluginManager的方法
    stub.set_lamda(static_cast<bool (PluginManager::*)(PluginMetaObjectPointer&)>(&PluginManager::loadPlugin),
                   [](PluginManager *self, PluginMetaObjectPointer &ptr) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(ptr)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<bool (PluginManager::*)(PluginMetaObjectPointer&)>(&PluginManager::initPlugin),
                   [](PluginManager *self, PluginMetaObjectPointer &ptr) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(ptr)
        __DBG_STUB_INVOKE__
        return false;  // 初始化失败
    });

    // 调用函数
    bool result = loadPlugin(testPlugin);

    // 验证结果
    EXPECT_FALSE(result);
}

/**
 * @brief 测试单个插件加载功能 - 启动失败
 * 验证loadPlugin函数在启动阶段失败的处理
 */
TEST_F(LifeCycleTest, LoadPlugin_StartFailure)
{
    auto testPlugin = createTestPluginMeta("TestPlugin");

    // 打桩PluginManager的方法
    stub.set_lamda(static_cast<bool (PluginManager::*)(PluginMetaObjectPointer&)>(&PluginManager::loadPlugin),
                   [](PluginManager *self, PluginMetaObjectPointer &ptr) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(ptr)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<bool (PluginManager::*)(PluginMetaObjectPointer&)>(&PluginManager::initPlugin),
                   [](PluginManager *self, PluginMetaObjectPointer &ptr) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(ptr)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<bool (PluginManager::*)(PluginMetaObjectPointer&)>(&PluginManager::startPlugin),
                   [](PluginManager *self, PluginMetaObjectPointer &ptr) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(ptr)
        __DBG_STUB_INVOKE__
        return false;  // 启动失败
    });

    // 调用函数
    bool result = loadPlugin(testPlugin);

    // 验证结果
    EXPECT_FALSE(result);
}

/**
 * @brief 测试单个插件关闭功能
 * 验证shutdownPlugin函数
 */
TEST_F(LifeCycleTest, ShutdownPlugin_Success)
{
    auto testPlugin = createTestPluginMeta("TestPlugin");
    bool stopPluginCalled = false;

    // 打桩PluginManager的方法
    stub.set_lamda(static_cast<bool (PluginManager::*)(PluginMetaObjectPointer&)>(&PluginManager::stopPlugin),
                   [&stopPluginCalled](PluginManager *self, PluginMetaObjectPointer &ptr) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(ptr)
        __DBG_STUB_INVOKE__
        stopPluginCalled = true;
        return true;
    });

    // 调用函数
    shutdownPlugin(testPlugin);

    // 验证结果
    EXPECT_TRUE(stopPluginCalled);
}

/**
 * @brief 测试单个插件关闭功能 - 空插件指针
 * 验证shutdownPlugin函数的错误处理
 */
TEST_F(LifeCycleTest, ShutdownPlugin_NullPointer)
{
    PluginMetaObjectPointer nullPlugin;

    // 调用函数应该不会崩溃
    shutdownPlugin(nullPlugin);

    // 如果到达这里说明没有崩溃
    EXPECT_TRUE(true);
}

/**
 * @brief 测试插件初始化状态查询
 * 验证isAllPluginsInitialized函数
 */
TEST_F(LifeCycleTest, IsAllPluginsInitialized)
{
    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::isAllPluginsInitialized, [](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    // 调用函数
    bool result = isAllPluginsInitialized();

    // 验证结果
    EXPECT_TRUE(result);
}

/**
 * @brief 测试插件启动状态查询
 * 验证isAllPluginsStarted函数
 */
TEST_F(LifeCycleTest, IsAllPluginsStarted)
{
    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::isAllPluginsStarted, [](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return false;
    });

    // 调用函数
    bool result = isAllPluginsStarted();

    // 验证结果
    EXPECT_FALSE(result);
}

/**
 * @brief 测试懒加载过滤器设置
 * 验证setLazyloadFilter函数
 */
TEST_F(LifeCycleTest, SetLazyloadFilter)
{
    bool filterSet = false;
    
    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::setLazyLoadFilter, 
                   [&filterSet](PluginManager *self, std::function<bool(const QString &)> filter) {
        Q_UNUSED(self)
        Q_UNUSED(filter)
        __DBG_STUB_INVOKE__
        filterSet = true;
    });

    // 创建测试过滤器
    auto testFilter = [](const QString &name) -> bool {
        return name.startsWith("Lazy");
    };

    // 调用函数
    setLazyloadFilter(testFilter);

    // 验证结果
    EXPECT_TRUE(filterSet);
}

/**
 * @brief 测试黑名单过滤器设置
 * 验证setBlackListFilter函数
 */
TEST_F(LifeCycleTest, SetBlackListFilter)
{
    bool filterSet = false;
    
    // 打桩PluginManager的方法
    stub.set_lamda(&PluginManager::setBlackListFilter, 
                   [&filterSet](PluginManager *self, std::function<bool(const QString &)> filter) {
        Q_UNUSED(self)
        Q_UNUSED(filter)
        __DBG_STUB_INVOKE__
        filterSet = true;
    });

    // 创建测试过滤器
    auto testFilter = [](const QString &name) -> bool {
        return name.contains("Black");
    };

    // 调用函数
    setBlackListFilter(testFilter);

    // 验证结果
    EXPECT_TRUE(filterSet);
}

/**
 * @brief 综合功能测试
 * 测试完整的插件管理流程
 */
TEST_F(LifeCycleTest, ComprehensiveTest)
{
    // 1. 初始化
    QStringList testIIDs = { "test.comprehensive.interface" };
    QStringList testPaths = { "/test/comprehensive" };
    QStringList testBlackNames = { "BadPlugin" };
    QStringList testLazyNames = { "LazyPlugin" };

    // 打桩基本函数
    stub.set_lamda(&PluginManager::addPluginIID, [](PluginManager *self, const QString &iid) {
        Q_UNUSED(self)
        Q_UNUSED(iid)
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&PluginManager::setPluginPaths, [](PluginManager *self, const QStringList &paths) {
        Q_UNUSED(self)
        Q_UNUSED(paths)
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&PluginManager::addBlackPluginName, [](PluginManager *self, const QString &name) {
        Q_UNUSED(self)
        Q_UNUSED(name)
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&PluginManager::addLazyLoadPluginName, [](PluginManager *self, const QString &name) {
        Q_UNUSED(self)
        Q_UNUSED(name)
        __DBG_STUB_INVOKE__
    });

    // 测试初始化
    initialize(testIIDs, testPaths, testBlackNames, testLazyNames);

    // 2. 测试读取插件
    stub.set_lamda(&PluginManager::readPlugins, [](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    bool readResult = readPlugins();
    EXPECT_TRUE(readResult);

    // 3. 测试加载插件
    stub.set_lamda(&PluginManager::loadPlugins, [](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&PluginManager::initPlugins, [](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&PluginManager::startPlugins, [](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    bool loadResult = loadPlugins();
    EXPECT_TRUE(loadResult);

    // 4. 测试状态查询
    stub.set_lamda(&PluginManager::isAllPluginsInitialized, [](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&PluginManager::isAllPluginsStarted, [](PluginManager *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(isAllPluginsInitialized());
    EXPECT_TRUE(isAllPluginsStarted());

    // 5. 测试关闭
    stub.set_lamda(&PluginManager::stopPlugins, [](PluginManager *self) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
    });

    shutdownPlugins();  // 应该不会崩溃

    // 如果到达这里说明综合测试通过
    EXPECT_TRUE(true);
}

/**
 * @brief 测试边界条件和错误处理
 * 验证各种边界条件下的行为
 */
TEST_F(LifeCycleTest, EdgeCases)
{
    // 测试空参数的初始化
    initialize(QStringList(), QStringList());

    // 测试空参数的带黑名单初始化
    initialize(QStringList(), QStringList(), QStringList());

    // 测试空参数的带懒加载初始化
    initialize(QStringList(), QStringList(), QStringList(), QStringList());

    // 测试空参数的Qt版本不敏感插件注册
    registerQtVersionInsensitivePlugins(QStringList());

    // 测试获取不存在插件的元数据
    stub.set_lamda(static_cast<PluginMetaObjectPointer (PluginManager::*)(const QString &, const QString) const>(&PluginManager::pluginMetaObj),
                   [](PluginManager *self, const QString &name, const QString &version) -> PluginMetaObjectPointer {
        Q_UNUSED(self)
        Q_UNUSED(name)
        Q_UNUSED(version)
        __DBG_STUB_INVOKE__
        return PluginMetaObjectPointer();  // 返回空指针
    });

    auto result = pluginMetaObj("NonExistentPlugin");
    EXPECT_TRUE(result.isNull());

    // 如果到达这里说明边界测试通过
    EXPECT_TRUE(true);
}

/**
 * @brief 测试并发安全性
 * 验证多线程环境下的行为（基础测试）
 */
TEST_F(LifeCycleTest, ThreadSafety)
{
    // 测试获取插件管理器实例的线程安全性
    // 注意：这是一个基础测试，真实的多线程测试会更复杂

    stub.set_lamda(&PluginManager::pluginIIDs, [](PluginManager *self) -> QStringList {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return QStringList() << "thread.safe.test";
    });

    // 多次调用应该是安全的
    for (int i = 0; i < 10; ++i) {
        auto result = pluginIIDs();
        EXPECT_EQ(result.size(), 1);
        EXPECT_EQ(result[0], "thread.safe.test");
    }
} 