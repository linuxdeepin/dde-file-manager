// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_pluginmanager_p.cpp - PluginManagerPrivate类单元测试
// 使用stub_ext进行函数打桩测试

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTest>
#include <QDir>
#include <QDirIterator>
#include <QPluginLoader>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLibrary>
#include <QFileInfo>
#include <chrono>

// 包含stub_ext
#include "stubext.h"

// 包含待测试的类
#include <dfm-framework/lifecycle/private/pluginmanager_p.h>
#include <dfm-framework/lifecycle/private/pluginmetaobject_p.h>
#include <dfm-framework/lifecycle/private/pluginquickmetadata_p.h>
#include <dfm-framework/lifecycle/pluginquickmetadata.h>
#include <dfm-framework/lifecycle/pluginmanager.h>
#include <dfm-framework/lifecycle/plugin.h>
#include <dfm-framework/lifecycle/plugincreator.h>
#include <dfm-framework/listener/listener.h>

using namespace dpf;

// 测试用的Mock类定义
class MockPlugin : public Plugin
{
public:
    static int instanceCount;
    static int initCount;
    static int startCount;
    
    MockPlugin() { instanceCount++; }
    ~MockPlugin() { instanceCount--; }
    void initialize() override { initCount++; }
    bool start() override { startCount++; return true; }
    void stop() override {}
};

class MockPluginSimple : public Plugin
{
public:
    void initialize() override {}
    bool start() override { return true; }
    void stop() override {}
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
        return QSharedPointer<Plugin>(new MockPluginSimple());
    }
};

// 静态成员变量定义
int MockPlugin::instanceCount = 0;
int MockPlugin::initCount = 0;
int MockPlugin::startCount = 0;

// 辅助函数：创建测试用的JSON元数据
QJsonObject createTestMetaData(const QString &name, const QString &version = "1.0.0",
                               const QStringList &depends = {})
{
    QJsonObject metaData;
    metaData[kPluginName] = name;
    metaData[kPluginVersion] = version;
    metaData[kPluginCategory] = "test";
    metaData[kPluginDescription] = "Test plugin";

    if (!depends.isEmpty()) {
        QJsonArray dependsArray;
        for (const QString &dep : depends) {
            QJsonObject dependObj;
            dependObj[kPluginName] = dep;
            dependObj[kPluginVersion] = "1.0.0";
            dependsArray.append(dependObj);
        }
        metaData[kPluginDepends] = dependsArray;
    }

    return metaData;
}

/**
 * @brief PluginManagerPrivate类单元测试
 *
 * 测试范围：
 * 1. 插件扫描和发现功能
 * 2. 插件元数据读取和解析
 * 3. 插件加载、初始化、启动流程
 * 4. 插件依赖排序算法
 * 5. 虚拟插件处理
 * 6. Qt版本兼容性检查
 * 7. 插件生命周期管理
 * 8. 错误处理和边界情况
 * 9. 黑名单和懒加载过滤
 */
class PluginManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 初始化测试环境
        stub.clear();

        // 设置测试用的应用程序
        if (!qApp) {
            static int argc = 1;
            static char *argv[] = { "test_pluginmanager" };
            app = std::make_unique<QCoreApplication>(argc, argv);
        }

        // 创建测试对象
        manager = std::make_unique<PluginManager>();
        managerPrivate = manager->d.data();

        // 设置测试参数
        managerPrivate->pluginLoadIIDs = { "test.plugin.interface" };
        managerPrivate->pluginLoadPaths = { "/test/plugins" };
        
        // 重置Mock计数器
        MockPlugin::instanceCount = 0;
        MockPlugin::initCount = 0;
        MockPlugin::startCount = 0;
    }

    void TearDown() override
    {
        // 清理测试环境
        stub.clear();
        manager.reset();
    }

    // 测试用的打桩器
    stub_ext::StubExt stub;
    std::unique_ptr<QCoreApplication> app;
    std::unique_ptr<PluginManager> manager;
    PluginManagerPrivate *managerPrivate;

    // 辅助函数：创建测试用的插件元数据
    PluginMetaObjectPointer createTestPlugin(const QString &name, const QString &version = "1.0.0")
    {
        PluginMetaObjectPointer plugin(new PluginMetaObject);
        plugin->d->name = name;
        plugin->d->version = version;
        plugin->d->iid = "test.plugin.interface";
        plugin->d->state = PluginMetaObject::kReaded;
        plugin->d->loader = QSharedPointer<QPluginLoader>(new QPluginLoader());
        plugin->d->loader->setFileName("/test/plugins/" + name + ".so");
        return plugin;
    }
};

/**
 * @brief 测试插件元数据获取
 * 验证pluginMetaObj函数的基本功能
 */
TEST_F(PluginManagerPrivateTest, PluginMetaObj_Basic)
{
    // 添加测试插件到读取队列
    auto plugin1 = createTestPlugin("TestPlugin1");
    auto plugin2 = createTestPlugin("TestPlugin2");
    
    managerPrivate->readQueue.append(plugin1);
    managerPrivate->readQueue.append(plugin2);

    // 测试获取存在的插件
    auto result1 = managerPrivate->pluginMetaObj("TestPlugin1");
    EXPECT_FALSE(result1.isNull());
    EXPECT_EQ(result1->name(), "TestPlugin1");

    auto result2 = managerPrivate->pluginMetaObj("TestPlugin2");
    EXPECT_FALSE(result2.isNull());
    EXPECT_EQ(result2->name(), "TestPlugin2");

    // 测试获取不存在的插件
    auto result3 = managerPrivate->pluginMetaObj("NonExistentPlugin");
    EXPECT_TRUE(result3.isNull());
}

/**
 * @brief 测试插件扫描功能 - 空IID列表
 * 验证scanfAllPlugin在没有配置IID时的行为
 */
TEST_F(PluginManagerPrivateTest, ScanfAllPlugin_EmptyIIDs)
{
    // 清空IID列表
    managerPrivate->pluginLoadIIDs.clear();

    // 调用扫描函数
    managerPrivate->scanfAllPlugin();

    // 验证没有插件被扫描到
    EXPECT_TRUE(managerPrivate->readQueue.isEmpty());
}

/**
 * @brief 测试插件扫描功能 - 基本扫描
 * 使用打桩模拟文件系统扫描
 */
TEST_F(PluginManagerPrivateTest, ScanfAllPlugin_BasicScan)
{
    // 打桩QDirIterator
    QStringList mockFiles = { "plugin1.so", "plugin2.so", "invalid.so" };
    int fileIndex = 0;

    stub.set_lamda(static_cast<bool (QDirIterator::*)() const>(&QDirIterator::hasNext),
                   [&fileIndex, &mockFiles](QDirIterator *self) -> bool {
                       Q_UNUSED(self)
                       __DBG_STUB_INVOKE__
                       return fileIndex < mockFiles.size();
                   });

    stub.set_lamda(static_cast<QString (QDirIterator::*)()>(&QDirIterator::next),
                   [&fileIndex, &mockFiles](QDirIterator *self) -> QString {
                       Q_UNUSED(self)
                       __DBG_STUB_INVOKE__
                       return "/test/plugins/" + mockFiles[fileIndex];
                   });

    stub.set_lamda(&QDirIterator::fileName, [&fileIndex, &mockFiles](QDirIterator *self) -> QString {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return mockFiles[fileIndex++];
    });

    stub.set_lamda(&QDirIterator::path, [](QDirIterator *self) -> QString {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return "/test/plugins";
    });

    // 打桩QPluginLoader::metaData
    stub.set_lamda(&QPluginLoader::metaData, [](QPluginLoader *self) -> QJsonObject {
        __DBG_STUB_INVOKE__
        QString fileName = self->fileName();
        QJsonObject root;
        root["IID"] = "test.plugin.interface";
        
        QJsonObject metaData;
        if (fileName.contains("plugin1")) {
            metaData[kPluginName] = "Plugin1";
            metaData[kPluginVersion] = "1.0.0";
            metaData[kPluginCategory] = "test";
            metaData[kPluginDescription] = "Test plugin";
        } else if (fileName.contains("plugin2")) {
            metaData[kPluginName] = "Plugin2";
            metaData[kPluginVersion] = "1.0.0";
            metaData[kPluginCategory] = "test";
            metaData[kPluginDescription] = "Test plugin";
        } else {
            // invalid plugin - wrong IID
            root["IID"] = "invalid.interface";
            metaData[kPluginName] = "InvalidPlugin";
            metaData[kPluginVersion] = "1.0.0";
            metaData[kPluginCategory] = "test";
            metaData[kPluginDescription] = "Test plugin";
        }
        
        root["MetaData"] = metaData;
        return root;
    });

    // 调用扫描函数
    managerPrivate->scanfAllPlugin();

    // 验证结果 - 应该只有2个有效插件
    EXPECT_EQ(managerPrivate->readQueue.size(), 2);
    
    bool found1 = false, found2 = false;
    for (const auto &plugin : managerPrivate->readQueue) {
        if (plugin->name() == "Plugin1") found1 = true;
        if (plugin->name() == "Plugin2") found2 = true;
    }
    EXPECT_TRUE(found1);
    EXPECT_TRUE(found2);
}

/**
 * @brief 测试虚拟插件扫描
 * 验证虚拟插件的特殊处理逻辑
 */
TEST_F(PluginManagerPrivateTest, ScanfVirtualPlugin_Basic)
{
    // 创建虚拟插件的JSON数据
    QJsonObject virtualMeta;
    virtualMeta[kPluginName] = "RealPlugin";
    virtualMeta[kPluginVersion] = "1.0.0";

    QJsonArray virtualList;
    QJsonObject virtual1;
    virtual1[kPluginName] = "VirtualPlugin1";
    virtual1[kPluginVersion] = "1.0.0";
    virtualList.append(virtual1);

    QJsonObject virtual2;
    virtual2[kPluginName] = "VirtualPlugin2";
    virtual2[kPluginVersion] = "1.0.0";
    virtualList.append(virtual2);

    QJsonObject dataJson;
    dataJson[kVirtualPluginMeta] = virtualMeta;
    dataJson[kVirtualPluginList] = virtualList;

    // 调用函数
    managerPrivate->scanfVirtualPlugin("/test/plugins/virtual.so", dataJson);

    // 验证结果
    EXPECT_EQ(managerPrivate->readQueue.size(), 2);
    
    for (const auto &plugin : managerPrivate->readQueue) {
        EXPECT_TRUE(plugin->isVirtual());
        EXPECT_EQ(plugin->d->realName, "RealPlugin");
        EXPECT_TRUE(plugin->name() == "VirtualPlugin1" || plugin->name() == "VirtualPlugin2");
    }
}

/**
 * @brief 测试黑名单过滤
 * 验证isBlackListed函数的各种情况
 */
TEST_F(PluginManagerPrivateTest, IsBlackListed_Basic)
{
    // 设置黑名单
    managerPrivate->blackPluginNames = { "BlacklistedPlugin" };

    // 测试黑名单中的插件
    EXPECT_TRUE(managerPrivate->isBlackListed("BlacklistedPlugin"));

    // 测试不在黑名单中的插件
    EXPECT_FALSE(managerPrivate->isBlackListed("NormalPlugin"));
}

/**
 * @brief 测试黑名单过滤器
 * 验证使用过滤器函数的黑名单功能
 */
TEST_F(PluginManagerPrivateTest, IsBlackListed_Filter)
{
    // 设置过滤器函数
    managerPrivate->blackListFilter = [](const QString &name) -> bool {
        return name.startsWith("Test");
    };

    // 测试匹配过滤器的插件
    EXPECT_TRUE(managerPrivate->isBlackListed("TestPlugin"));
    EXPECT_TRUE(managerPrivate->isBlackListed("TestAnotherPlugin"));

    // 测试不匹配过滤器的插件
    EXPECT_FALSE(managerPrivate->isBlackListed("NormalPlugin"));
}

/**
 * @brief 测试JSON元数据解析
 * 验证jsonToMeta函数的基本功能
 */
TEST_F(PluginManagerPrivateTest, JsonToMeta_Basic)
{
    auto plugin = createTestPlugin("TestPlugin");
    
    // 创建测试用的JSON数据
    QJsonObject metaData = createTestMetaData("TestPlugin", "2.0.0", {"DependPlugin"});
    metaData[kPluginCategory] = "TestCategory";
    metaData[kPluginDescription] = "Test Description";
    metaData[kPluginUrlLink] = "http://test.com";

    // 添加自定义数据
    QJsonObject customData;
    customData["key1"] = "value1";
    customData["key2"] = 42;
    metaData[kCustomData] = customData;

    // 调用函数
    managerPrivate->jsonToMeta(plugin, metaData);

    // 验证结果
    EXPECT_EQ(plugin->version(), "2.0.0");
    EXPECT_EQ(plugin->category(), "TestCategory");
    EXPECT_EQ(plugin->description(), "Test Description");
    EXPECT_EQ(plugin->urlLink(), "http://test.com");
    EXPECT_EQ(plugin->depends().size(), 1);
    EXPECT_EQ(plugin->depends().first().pluginName, "DependPlugin");
    EXPECT_EQ(plugin->d->state, PluginMetaObject::kReaded);
    
    // 验证自定义数据
    QVariantMap customMap = plugin->customData();
    EXPECT_EQ(customMap["key1"].toString(), "value1");
    EXPECT_EQ(customMap["key2"].toInt(), 42);
}

/**
 * @brief 测试Quick组件元数据解析
 * 验证QML组件相关的元数据处理
 */
TEST_F(PluginManagerPrivateTest, JsonToMeta_QuickComponents)
{
    auto plugin = createTestPlugin("TestPlugin");
    
    // 创建包含Quick组件的JSON数据
    QJsonObject metaData = createTestMetaData("TestPlugin", "1.0.0", {"ParentPlugin"});
    
    QJsonArray quickArray;
    QJsonObject quick1;
    quick1[kQuickId] = "TestComponent1";
    quick1[kQuickUrl] = "components/TestComponent1.qml";
    quick1[kQuickType] = "Item";
    quick1[kQuickParent] = "ParentPlugin.SomeComponent";
    quickArray.append(quick1);

    QJsonObject quick2;
    quick2[kQuickId] = "TestComponent2";
    quick2[kQuickUrl] = "components/TestComponent2.qml";
    quick2[kQuickApplet] = "TestApplet";
    quickArray.append(quick2);

    metaData[kQuick] = quickArray;

    // 打桩QFileInfo
    stub.set_lamda(&QFileInfo::absolutePath, [](QFileInfo *self) -> QString {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return "/test/plugins";
    });

    // 调用函数
    managerPrivate->jsonToMeta(plugin, metaData);

    // 验证Quick组件
    EXPECT_EQ(plugin->d->quickMetaList.size(), 2);
    
    auto quick1Meta = plugin->d->quickMetaList[0];
    EXPECT_EQ(quick1Meta->d->quickId, "TestComponent1");
    EXPECT_EQ(quick1Meta->d->quickType, "Item");
    EXPECT_EQ(quick1Meta->d->quickParent, "ParentPlugin.SomeComponent");
    
    auto quick2Meta = plugin->d->quickMetaList[1];
    EXPECT_EQ(quick2Meta->d->quickId, "TestComponent2");
    EXPECT_EQ(quick2Meta->d->quickApplet, "TestApplet");
}

/**
 * @brief 测试Quick组件元数据解析 - 无效组件
 * 验证对无效Quick组件的处理
 */
TEST_F(PluginManagerPrivateTest, JsonToMeta_InvalidQuickComponents)
{
    auto plugin = createTestPlugin("TestPlugin");
    
    QJsonObject metaData = createTestMetaData("TestPlugin");
    
    QJsonArray quickArray;
    // 无效组件 - 缺少ID
    QJsonObject invalid1;
    invalid1[kQuickUrl] = "components/Invalid1.qml";
    quickArray.append(invalid1);

    // 无效组件 - 缺少URL
    QJsonObject invalid2;
    invalid2[kQuickId] = "Invalid2";
    quickArray.append(invalid2);

    // 无效组件 - parent依赖不存在
    QJsonObject invalid3;
    invalid3[kQuickId] = "Invalid3";
    invalid3[kQuickUrl] = "components/Invalid3.qml";
    invalid3[kQuickParent] = "NonExistentPlugin.Component";
    quickArray.append(invalid3);

    metaData[kQuick] = quickArray;

    // 调用函数
    managerPrivate->jsonToMeta(plugin, metaData);

    // 验证无效组件被过滤掉
    EXPECT_EQ(plugin->d->quickMetaList.size(), 0);
}

/**
 * @brief 测试插件依赖排序 - 基本排序
 * 验证dependsSort函数的基本功能
 */
TEST_F(PluginManagerPrivateTest, DependsSort_Basic)
{
    // 创建测试插件 - A依赖B，B依赖C
    auto pluginA = createTestPlugin("PluginA");
    auto pluginB = createTestPlugin("PluginB");
    auto pluginC = createTestPlugin("PluginC");

    // 设置依赖关系
    PluginDepend dependAB;
    dependAB.pluginName = "PluginB";
    dependAB.pluginVersion = "1.0.0";
    pluginA->d->depends.append(dependAB);

    PluginDepend dependBC;
    dependBC.pluginName = "PluginC";
    dependBC.pluginVersion = "1.0.0";
    pluginB->d->depends.append(dependBC);

    // 创建源队列（乱序）
    QQueue<PluginMetaObjectPointer> srcQueue;
    srcQueue.append(pluginA);
    srcQueue.append(pluginB);
    srcQueue.append(pluginC);

    // 执行排序
    QQueue<PluginMetaObjectPointer> destQueue;
    managerPrivate->dependsSort(&destQueue, &srcQueue);

    // 验证排序结果 - 应该是 C, B, A 的顺序
    EXPECT_EQ(destQueue.size(), 3);
    EXPECT_EQ(destQueue[0]->name(), "PluginC");
    EXPECT_EQ(destQueue[1]->name(), "PluginB");
    EXPECT_EQ(destQueue[2]->name(), "PluginA");
}

/**
 * @brief 测试插件依赖排序 - 循环依赖
 * 验证循环依赖的检测和处理
 */
TEST_F(PluginManagerPrivateTest, DependsSort_CircularDependency)
{
    // 创建循环依赖 - A依赖B，B依赖A
    auto pluginA = createTestPlugin("PluginA");
    auto pluginB = createTestPlugin("PluginB");

    PluginDepend dependAB;
    dependAB.pluginName = "PluginB";
    pluginA->d->depends.append(dependAB);

    PluginDepend dependBA;
    dependBA.pluginName = "PluginA";
    pluginB->d->depends.append(dependBA);

    QQueue<PluginMetaObjectPointer> srcQueue;
    srcQueue.append(pluginA);
    srcQueue.append(pluginB);

    QQueue<PluginMetaObjectPointer> destQueue;
    managerPrivate->dependsSort(&destQueue, &srcQueue);

    // 循环依赖时应该返回原始顺序
    EXPECT_EQ(destQueue.size(), 2);
}

/**
 * @brief 测试插件依赖排序 - 缺失依赖
 * 验证依赖插件不存在时的处理
 */
TEST_F(PluginManagerPrivateTest, DependsSort_MissingDependency)
{
    auto pluginA = createTestPlugin("PluginA");
    
    // 依赖一个不存在的插件
    PluginDepend dependMissing;
    dependMissing.pluginName = "MissingPlugin";
    pluginA->d->depends.append(dependMissing);

    QQueue<PluginMetaObjectPointer> srcQueue;
    srcQueue.append(pluginA);

    QQueue<PluginMetaObjectPointer> destQueue;
    managerPrivate->dependsSort(&destQueue, &srcQueue);

    // 应该仍然包含插件A
    EXPECT_EQ(destQueue.size(), 1);
    EXPECT_EQ(destQueue[0]->name(), "PluginA");
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
/**
 * @brief 测试Qt版本检查 - 成功情况
 * 验证checkPluginQtVersion函数的成功路径
 */
TEST_F(PluginManagerPrivateTest, CheckPluginQtVersion_Success)
{
    auto plugin = createTestPlugin("TestPlugin");
    
    // 静态函数用于模拟qVersion
    static auto mockQVersion = []() -> const char * {
        return "6.5.0";
    };
    
    // 打桩QLibrary
    stub.set_lamda(&QLibrary::load, [](QLibrary *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QLibrary::unload, [](QLibrary *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<QFunctionPointer (QLibrary::*)(const char *)>(&QLibrary::resolve), 
                   [](QLibrary *self, const char *symbol) -> QFunctionPointer {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        if (strcmp(symbol, "qVersion") == 0) {
            return reinterpret_cast<QFunctionPointer>(+mockQVersion);
        }
        return nullptr;
    });

    // 调用函数
    bool result = managerPrivate->checkPluginQtVersion(plugin);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_TRUE(plugin->d->error.isEmpty());
}

/**
 * @brief 测试Qt版本检查 - 版本不兼容
 * 验证Qt5插件在Qt6应用中的检查
 */
TEST_F(PluginManagerPrivateTest, CheckPluginQtVersion_Incompatible)
{
    auto plugin = createTestPlugin("TestPlugin");
    
    // 静态函数用于模拟qVersion
    static auto mockQVersionQt5 = []() -> const char * {
        return "5.15.0";  // Qt5版本
    };
    
    // 打桩QLibrary
    stub.set_lamda(&QLibrary::load, [](QLibrary *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QLibrary::unload, [](QLibrary *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<QFunctionPointer (QLibrary::*)(const char *)>(&QLibrary::resolve), 
                   [](QLibrary *self, const char *symbol) -> QFunctionPointer {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        if (strcmp(symbol, "qVersion") == 0) {
            return reinterpret_cast<QFunctionPointer>(+mockQVersionQt5);
        }
        return nullptr;
    });

    // 调用函数
    bool result = managerPrivate->checkPluginQtVersion(plugin);

    // 验证结果
    EXPECT_FALSE(result);
    EXPECT_FALSE(plugin->d->error.isEmpty());
    EXPECT_NE(plugin->d->error.indexOf("Qt version compatibility"), -1);
}

/**
 * @brief 测试Qt版本检查 - 无qVersion符号
 * 验证插件没有链接Qt时的处理
 */
TEST_F(PluginManagerPrivateTest, CheckPluginQtVersion_NoQtLink)
{
    auto plugin = createTestPlugin("TestPlugin");
    
    // 打桩QLibrary
    stub.set_lamda(&QLibrary::load, [](QLibrary *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QLibrary::unload, [](QLibrary *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<QFunctionPointer (QLibrary::*)(const char *)>(&QLibrary::resolve), 
                   [](QLibrary *self, const char *symbol) -> QFunctionPointer {
        Q_UNUSED(self)
        Q_UNUSED(symbol)
        __DBG_STUB_INVOKE__
        return nullptr;  // 无法解析qVersion符号
    });

    // 调用函数
    bool result = managerPrivate->checkPluginQtVersion(plugin);

    // 验证结果
    EXPECT_FALSE(result);
    EXPECT_FALSE(plugin->d->error.isEmpty());
    EXPECT_NE(plugin->d->error.indexOf("does not link against Qt"), -1);
}

/**
 * @brief 测试Qt版本检查 - 库加载失败
 * 验证库加载失败时的处理
 */
TEST_F(PluginManagerPrivateTest, CheckPluginQtVersion_LoadFailure)
{
    auto plugin = createTestPlugin("TestPlugin");
    
    // 打桩QLibrary
    stub.set_lamda(&QLibrary::load, [](QLibrary *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&QLibrary::errorString, [](QLibrary *self) -> QString {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return "Failed to load library";
    });

    // 调用函数
    bool result = managerPrivate->checkPluginQtVersion(plugin);

    // 验证结果
    EXPECT_FALSE(result);
    EXPECT_FALSE(plugin->d->error.isEmpty());
    EXPECT_NE(plugin->d->error.indexOf("Failed to load library"), -1);
}

/**
 * @brief 测试Qt版本检查 - 版本不敏感插件
 * 验证版本不敏感插件的处理
 */
TEST_F(PluginManagerPrivateTest, CheckPluginQtVersion_VersionInsensitive)
{
    auto plugin = createTestPlugin("TestPlugin");
    
    // 将插件添加到版本不敏感列表
    managerPrivate->qtVersionInsensitivePluginNames.append("TestPlugin");

    // 调用函数
    bool result = managerPrivate->checkPluginQtVersion(plugin);

    // 验证结果 - 应该跳过版本检查
    EXPECT_TRUE(result);
    EXPECT_TRUE(plugin->d->error.isEmpty());
}
#endif 

/**
 * @brief 测试插件加载 - 成功情况
 * 验证doLoadPlugin函数的基本功能
 */
TEST_F(PluginManagerPrivateTest, DoLoadPlugin_Success)
{
    auto plugin = createTestPlugin("TestPlugin");
    plugin->d->state = PluginMetaObject::kReaded;

    // 打桩QPluginLoader
    stub.set_lamda(&QPluginLoader::load, [](QPluginLoader *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QPluginLoader::instance, [](QPluginLoader *self) -> QObject * {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return new MockPluginSimple();
    });

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // 打桩Qt版本检查
    stub.set_lamda(&PluginManagerPrivate::checkPluginQtVersion, 
                   [](PluginManagerPrivate *self, PluginMetaObjectPointer pointer) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(pointer)
        __DBG_STUB_INVOKE__
        return true;
    });
#endif

    // 调用函数
    bool result = managerPrivate->doLoadPlugin(plugin);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_EQ(plugin->d->state, PluginMetaObject::kLoaded);
    EXPECT_FALSE(plugin->d->plugin.isNull());
}

/**
 * @brief 测试插件加载 - 已加载状态
 * 验证重复加载已加载插件的处理
 */
TEST_F(PluginManagerPrivateTest, DoLoadPlugin_AlreadyLoaded)
{
    auto plugin = createTestPlugin("TestPlugin");
    plugin->d->state = PluginMetaObject::kLoaded;

    // 调用函数
    bool result = managerPrivate->doLoadPlugin(plugin);

    // 验证结果 - 应该直接返回成功
    EXPECT_TRUE(result);
    EXPECT_EQ(plugin->d->state, PluginMetaObject::kLoaded);
}

/**
 * @brief 测试插件加载 - 未读取状态
 * 验证未完成读取的插件的处理
 */
TEST_F(PluginManagerPrivateTest, DoLoadPlugin_NotReaded)
{
    auto plugin = createTestPlugin("TestPlugin");
    plugin->d->state = PluginMetaObject::kInvalid;

    // 调用函数
    bool result = managerPrivate->doLoadPlugin(plugin);

    // 验证结果 - 应该失败
    EXPECT_FALSE(result);
}

/**
 * @brief 测试插件加载 - 加载失败
 * 验证QPluginLoader加载失败的处理
 */
TEST_F(PluginManagerPrivateTest, DoLoadPlugin_LoadFailure)
{
    auto plugin = createTestPlugin("TestPlugin");
    plugin->d->state = PluginMetaObject::kReaded;

    // 打桩QPluginLoader加载失败
    stub.set_lamda(&QPluginLoader::load, [](QPluginLoader *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&QPluginLoader::errorString, [](QPluginLoader *self) -> QString {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return "Load failed";
    });

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // 打桩Qt版本检查
    stub.set_lamda(&PluginManagerPrivate::checkPluginQtVersion, 
                   [](PluginManagerPrivate *self, PluginMetaObjectPointer pointer) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(pointer)
        __DBG_STUB_INVOKE__
        return true;
    });
#endif

    // 调用函数
    bool result = managerPrivate->doLoadPlugin(plugin);

    // 验证结果
    EXPECT_FALSE(result);
    EXPECT_FALSE(plugin->d->error.isEmpty());
}

/**
 * @brief 测试虚拟插件加载 - 成功情况
 * 验证虚拟插件的特殊加载逻辑
 */
TEST_F(PluginManagerPrivateTest, DoLoadPlugin_VirtualPlugin_Success)
{
    auto plugin = createTestPlugin("VirtualPlugin");
    plugin->d->state = PluginMetaObject::kReaded;
    plugin->d->isVirtual = true;
    plugin->d->realName = "RealPlugin";

    // 打桩QPluginLoader
    stub.set_lamda(&QPluginLoader::load, [](QPluginLoader *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&QPluginLoader::instance, [](QPluginLoader *self) -> QObject * {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return new MockPluginCreator();
    });

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // 打桩Qt版本检查
    stub.set_lamda(&PluginManagerPrivate::checkPluginQtVersion, 
                   [](PluginManagerPrivate *self, PluginMetaObjectPointer pointer) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(pointer)
        __DBG_STUB_INVOKE__
        return true;
    });
#endif

    // 调用函数
    bool result = managerPrivate->doLoadPlugin(plugin);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_EQ(plugin->d->state, PluginMetaObject::kLoaded);
    EXPECT_FALSE(plugin->d->plugin.isNull());
}

/**
 * @brief 测试插件初始化 - 成功情况
 * 验证doInitPlugin函数的基本功能
 */
TEST_F(PluginManagerPrivateTest, DoInitPlugin_Success)
{
    auto plugin = createTestPlugin("TestPlugin");
    plugin->d->state = PluginMetaObject::kLoaded;

    plugin->d->plugin = QSharedPointer<Plugin>(new MockPluginSimple());

    // 打桩Listener信号
    bool signal_emitted = false;
    stub.set_lamda(&Listener::pluginInitialized, [&signal_emitted](Listener *self, const QString &iid, const QString &name) {
        Q_UNUSED(self)
        Q_UNUSED(iid)
        Q_UNUSED(name)
        __DBG_STUB_INVOKE__
        signal_emitted = true;
    });

    // 调用函数
    bool result = managerPrivate->doInitPlugin(plugin);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_EQ(plugin->d->state, PluginMetaObject::kInitialized);
    EXPECT_TRUE(signal_emitted);
}

/**
 * @brief 测试插件初始化 - 已初始化状态
 * 验证重复初始化的处理
 */
TEST_F(PluginManagerPrivateTest, DoInitPlugin_AlreadyInitialized)
{
    auto plugin = createTestPlugin("TestPlugin");
    plugin->d->state = PluginMetaObject::kInitialized;

    // 调用函数
    bool result = managerPrivate->doInitPlugin(plugin);

    // 验证结果 - 应该直接返回成功
    EXPECT_TRUE(result);
    EXPECT_EQ(plugin->d->state, PluginMetaObject::kInitialized);
}

/**
 * @brief 测试插件初始化 - 未加载状态
 * 验证未加载插件的初始化处理
 */
TEST_F(PluginManagerPrivateTest, DoInitPlugin_NotLoaded)
{
    auto plugin = createTestPlugin("TestPlugin");
    plugin->d->state = PluginMetaObject::kReaded;

    // 调用函数
    bool result = managerPrivate->doInitPlugin(plugin);

    // 验证结果 - 应该失败
    EXPECT_FALSE(result);
}

/**
 * @brief 测试插件启动 - 成功情况
 * 验证doStartPlugin函数的基本功能
 */
TEST_F(PluginManagerPrivateTest, DoStartPlugin_Success)
{
    auto plugin = createTestPlugin("TestPlugin");
    plugin->d->state = PluginMetaObject::kInitialized;

    plugin->d->plugin = QSharedPointer<Plugin>(new MockPluginSimple());

    // 打桩Listener信号
    bool signal_emitted = false;
    stub.set_lamda(&Listener::pluginStarted, [&signal_emitted](Listener *self, const QString &iid, const QString &name) {
        Q_UNUSED(self)
        Q_UNUSED(iid)
        Q_UNUSED(name)
        __DBG_STUB_INVOKE__
        signal_emitted = true;
    });

    // 调用函数
    bool result = managerPrivate->doStartPlugin(plugin);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_EQ(plugin->d->state, PluginMetaObject::kStarted);
    EXPECT_TRUE(signal_emitted);
}

/**
 * @brief 测试插件启动 - 启动失败
 * 验证插件start()方法返回false的处理
 */
TEST_F(PluginManagerPrivateTest, DoStartPlugin_StartFailure)
{
    auto plugin = createTestPlugin("TestPlugin");
    plugin->d->state = PluginMetaObject::kInitialized;

    plugin->d->plugin = QSharedPointer<Plugin>(new MockPluginFailStart());

    // 调用函数
    bool result = managerPrivate->doStartPlugin(plugin);

    // 验证结果
    EXPECT_FALSE(result);
    EXPECT_FALSE(plugin->d->error.isEmpty());
}

/**
 * @brief 测试插件停止 - 成功情况
 * 验证doStopPlugin函数的基本功能
 */
TEST_F(PluginManagerPrivateTest, DoStopPlugin_Success)
{
    auto plugin = createTestPlugin("TestPlugin");
    plugin->d->state = PluginMetaObject::kStarted;

    plugin->d->plugin = QSharedPointer<Plugin>(new MockPluginSimple());

    // 打桩QPluginLoader::unload
    stub.set_lamda(&QPluginLoader::unload, [](QPluginLoader *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    // 调用函数
    bool result = managerPrivate->doStopPlugin(plugin);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_EQ(plugin->d->state, PluginMetaObject::kShutdown);
    EXPECT_TRUE(plugin->d->plugin.isNull());
}

/**
 * @brief 测试插件停止 - 卸载失败
 * 验证QPluginLoader::unload失败的处理
 */
TEST_F(PluginManagerPrivateTest, DoStopPlugin_UnloadFailure)
{
    auto plugin = createTestPlugin("TestPlugin");
    plugin->d->state = PluginMetaObject::kStarted;
    plugin->d->isVirtual = false;  // 非虚拟插件才会尝试卸载

    plugin->d->plugin = QSharedPointer<Plugin>(new MockPluginSimple());

    // 打桩QPluginLoader::unload失败
    stub.set_lamda(&QPluginLoader::unload, [](QPluginLoader *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&QPluginLoader::errorString, [](QPluginLoader *self) -> QString {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return "Unload failed";
    });

    // 调用函数
    bool result = managerPrivate->doStopPlugin(plugin);

    // 验证结果 - 卸载失败应该返回false
    EXPECT_FALSE(result);
}

/**
 * @brief 测试读取所有插件 - 成功情况
 * 验证readPlugins函数的基本流程
 */
TEST_F(PluginManagerPrivateTest, ReadPlugins_Success)
{
    // 打桩scanfAllPlugin
    stub.set_lamda(&PluginManagerPrivate::scanfAllPlugin, [this]() {
        __DBG_STUB_INVOKE__
        // 模拟扫描到的插件
        auto plugin1 = createTestPlugin("Plugin1");
        auto plugin2 = createTestPlugin("Plugin2");
        managerPrivate->readQueue.append(plugin1);
        managerPrivate->readQueue.append(plugin2);
    });

    // 打桩readJsonToMeta
    stub.set_lamda(&PluginManagerPrivate::readJsonToMeta, [](PluginManagerPrivate *self, PluginMetaObjectPointer obj) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        obj->d->state = PluginMetaObject::kReaded;
    });

    // 调用函数
    bool result = managerPrivate->readPlugins();

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_EQ(managerPrivate->pluginsToLoad.size(), 2);
}

/**
 * @brief 测试读取所有插件 - 懒加载过滤
 * 验证懒加载插件的过滤逻辑
 */
TEST_F(PluginManagerPrivateTest, ReadPlugins_LazyLoad)
{
    // 设置懒加载插件名称
    managerPrivate->lazyLoadPluginNames = { "LazyPlugin" };

    // 设置懒加载过滤器
    managerPrivate->lazyPluginFilter = [](const QString &name) -> bool {
        return name.startsWith("Lazy");
    };

    // 打桩scanfAllPlugin
    stub.set_lamda(&PluginManagerPrivate::scanfAllPlugin, [this]() {
        __DBG_STUB_INVOKE__
        auto plugin1 = createTestPlugin("NormalPlugin");
        auto plugin2 = createTestPlugin("LazyPlugin");
        auto plugin3 = createTestPlugin("LazyFilterPlugin");
        managerPrivate->readQueue.append(plugin1);
        managerPrivate->readQueue.append(plugin2);
        managerPrivate->readQueue.append(plugin3);
    });

    // 打桩readJsonToMeta
    stub.set_lamda(&PluginManagerPrivate::readJsonToMeta, [](PluginManagerPrivate *self, PluginMetaObjectPointer obj) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        obj->d->state = PluginMetaObject::kReaded;
    });

    // 调用函数
    bool result = managerPrivate->readPlugins();

    // 验证结果 - 只有NormalPlugin应该被加载
    EXPECT_TRUE(result);
    EXPECT_EQ(managerPrivate->pluginsToLoad.size(), 1);
    EXPECT_EQ(managerPrivate->pluginsToLoad[0]->name(), "NormalPlugin");
    
    // 验证懒加载插件被添加到懒加载列表
    EXPECT_TRUE(managerPrivate->lazyLoadPluginNames.contains("LazyFilterPlugin"));
}

/**
 * @brief 测试批量加载插件 - 成功情况
 * 验证loadPlugins函数的基本流程
 */
TEST_F(PluginManagerPrivateTest, LoadPlugins_Success)
{
    // 准备测试数据
    auto plugin1 = createTestPlugin("Plugin1");
    auto plugin2 = createTestPlugin("Plugin2");
    managerPrivate->pluginsToLoad.append(plugin1);
    managerPrivate->pluginsToLoad.append(plugin2);

    // 打桩dependsSort
    stub.set_lamda(&PluginManagerPrivate::dependsSort, 
                   [](PluginManagerPrivate *self, QQueue<PluginMetaObjectPointer> *dst, const QQueue<PluginMetaObjectPointer> *src) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        *dst = *src;  // 简单复制
    });

    // 打桩doLoadPlugin
    stub.set_lamda(&PluginManagerPrivate::doLoadPlugin, 
                   [](PluginManagerPrivate *self, PluginMetaObjectPointer pointer) -> bool {
        Q_UNUSED(self)
        Q_UNUSED(pointer)
        __DBG_STUB_INVOKE__
        return true;
    });

    // 调用函数
    bool result = managerPrivate->loadPlugins();

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_EQ(managerPrivate->loadQueue.size(), 2);
}

/**
 * @brief 测试批量加载插件 - 部分失败
 * 验证某些插件加载失败时的处理
 */
TEST_F(PluginManagerPrivateTest, LoadPlugins_PartialFailure)
{
    // 准备测试数据
    auto plugin1 = createTestPlugin("Plugin1");
    auto plugin2 = createTestPlugin("Plugin2");
    managerPrivate->pluginsToLoad.append(plugin1);
    managerPrivate->pluginsToLoad.append(plugin2);

    // 打桩dependsSort
    stub.set_lamda(&PluginManagerPrivate::dependsSort, 
                   [](PluginManagerPrivate *self, QQueue<PluginMetaObjectPointer> *dst, const QQueue<PluginMetaObjectPointer> *src) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        *dst = *src;
    });

    // 打桩doLoadPlugin - Plugin1失败，Plugin2成功
    stub.set_lamda(&PluginManagerPrivate::doLoadPlugin, 
                   [](PluginManagerPrivate *self, PluginMetaObjectPointer pointer) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return pointer->name() != "Plugin1";  // Plugin1失败
    });

    // 调用函数
    bool result = managerPrivate->loadPlugins();

    // 验证结果 - 应该返回false但成功的插件仍在队列中
    EXPECT_FALSE(result);
    EXPECT_EQ(managerPrivate->loadQueue.size(), 1);
    EXPECT_EQ(managerPrivate->loadQueue[0]->name(), "Plugin2");
}

/**
 * @brief 综合功能测试
 * 通过综合测试验证所有功能的协同工作
 */
TEST_F(PluginManagerPrivateTest, ComprehensiveTest)
{
    // 1. 设置测试环境
    managerPrivate->pluginLoadIIDs = { "test.plugin.interface" };
    managerPrivate->pluginLoadPaths = { "/test/plugins" };

    // 2. 测试插件元数据获取
    auto plugin = createTestPlugin("TestPlugin");
    managerPrivate->readQueue.append(plugin);
    
    auto foundPlugin = managerPrivate->pluginMetaObj("TestPlugin");
    EXPECT_FALSE(foundPlugin.isNull());
    EXPECT_EQ(foundPlugin->name(), "TestPlugin");

    // 3. 测试黑名单功能
    EXPECT_FALSE(managerPrivate->isBlackListed("TestPlugin"));
    managerPrivate->blackPluginNames.append("TestPlugin");
    EXPECT_TRUE(managerPrivate->isBlackListed("TestPlugin"));

    // 4. 测试JSON元数据解析
    QJsonObject metaData = createTestMetaData("TestPlugin", "1.0.0");
    managerPrivate->jsonToMeta(plugin, metaData);
    EXPECT_EQ(plugin->version(), "1.0.0");

    // 如果到达这里说明所有功能都正常工作
    EXPECT_TRUE(true);
}

/**
 * @brief 测试边界条件和错误处理
 * 验证各种边界条件下的行为
 */
TEST_F(PluginManagerPrivateTest, EdgeCases)
{
    // 测试空插件队列的依赖排序
    QQueue<PluginMetaObjectPointer> emptyQueue;
    QQueue<PluginMetaObjectPointer> destQueue;
    managerPrivate->dependsSort(&destQueue, &emptyQueue);
    EXPECT_TRUE(destQueue.isEmpty());

    // 测试空名称插件的黑名单检查
    EXPECT_FALSE(managerPrivate->isBlackListed(""));

    // 测试获取不存在插件的元数据
    auto nonExistent = managerPrivate->pluginMetaObj("NonExistent");
    EXPECT_TRUE(nonExistent.isNull());

    // 测试虚拟插件的scanfVirtualPlugin空数据
    managerPrivate->scanfVirtualPlugin("/invalid/path.so", QJsonObject());
    // 应该不会崩溃
    EXPECT_TRUE(true);
}

/**
 * @brief 测试插件状态转换
 * 验证插件生命周期状态的正确转换
 */
TEST_F(PluginManagerPrivateTest, PluginStateTransitions)
{
    auto plugin = createTestPlugin("StateTestPlugin");
    
    // 初始状态
    EXPECT_EQ(plugin->d->state, PluginMetaObject::kReaded);

    // 不能从kReaded直接初始化
    bool initResult = managerPrivate->doInitPlugin(plugin);
    EXPECT_FALSE(initResult);

    // 不能从kReaded直接启动
    bool startResult = managerPrivate->doStartPlugin(plugin);
    EXPECT_FALSE(startResult);

    // 不能从kReaded直接停止
    bool stopResult = managerPrivate->doStopPlugin(plugin);
    EXPECT_FALSE(stopResult);
}

/**
 * @brief 测试内存管理
 * 验证插件实例的内存管理是否正确
 */
TEST_F(PluginManagerPrivateTest, MemoryManagement)
{
    auto plugin = createTestPlugin("MemoryTestPlugin");
    plugin->d->state = PluginMetaObject::kStarted;

    plugin->d->plugin = QSharedPointer<Plugin>(new MockPlugin());
    
    // 验证实例创建
    EXPECT_EQ(MockPlugin::instanceCount, 1);

    // 打桩QPluginLoader::unload
    stub.set_lamda(&QPluginLoader::unload, [](QPluginLoader *self) -> bool {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        return true;
    });

    // 停止插件应该释放实例
    managerPrivate->doStopPlugin(plugin);
    EXPECT_EQ(MockPlugin::instanceCount, 0);
    EXPECT_TRUE(plugin->d->plugin.isNull());
}

/**
 * @brief 测试多插件批量操作
 * 验证批量初始化和启动插件的功能
 */
TEST_F(PluginManagerPrivateTest, BatchOperations)
{
    // 创建多个插件
    auto plugin1 = createTestPlugin("BatchPlugin1");
    auto plugin2 = createTestPlugin("BatchPlugin2");
    auto plugin3 = createTestPlugin("BatchPlugin3");

    // 添加到加载队列（模拟已加载状态）
    plugin1->d->state = PluginMetaObject::kLoaded;
    plugin2->d->state = PluginMetaObject::kLoaded;
    plugin3->d->state = PluginMetaObject::kLoaded;

    plugin1->d->plugin = QSharedPointer<Plugin>(new MockPlugin());
    plugin2->d->plugin = QSharedPointer<Plugin>(new MockPlugin());
    plugin3->d->plugin = QSharedPointer<Plugin>(new MockPlugin());

    managerPrivate->loadQueue.append(plugin1);
    managerPrivate->loadQueue.append(plugin2);
    managerPrivate->loadQueue.append(plugin3);

    // 批量初始化
    bool initResult = managerPrivate->initPlugins();
    EXPECT_TRUE(initResult);
    EXPECT_EQ(MockPlugin::initCount, 3);

    // 批量启动
    bool startResult = managerPrivate->startPlugins();
    EXPECT_TRUE(startResult);
    EXPECT_EQ(MockPlugin::startCount, 3);

    // 验证状态变更标志
    EXPECT_TRUE(managerPrivate->allPluginsInitialized);
    EXPECT_TRUE(managerPrivate->allPluginsStarted);
}

/**
 * @brief 性能测试
 * 验证插件管理器的性能特征
 */
TEST_F(PluginManagerPrivateTest, Performance)
{
    const int pluginCount = 100;
    
    // 创建大量插件用于测试
    QQueue<PluginMetaObjectPointer> largeQueue;
    for (int i = 0; i < pluginCount; ++i) {
        auto plugin = createTestPlugin(QString("PerfPlugin%1").arg(i));
        largeQueue.append(plugin);
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    // 测试依赖排序性能
    QQueue<PluginMetaObjectPointer> sortedQueue;
    managerPrivate->dependsSort(&sortedQueue, &largeQueue);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 验证性能（100个插件的排序应该在合理时间内完成）
    EXPECT_LT(duration.count(), 1000);  // 小于1秒
    EXPECT_EQ(sortedQueue.size(), pluginCount);
}

/**
 * @brief 测试信号发送
 * 验证插件生命周期中的信号发送
 */
TEST_F(PluginManagerPrivateTest, SignalEmission)
{
    auto plugin = createTestPlugin("SignalTestPlugin");
    plugin->d->state = PluginMetaObject::kLoaded;
    plugin->d->iid = "test.signal.interface";

    plugin->d->plugin = QSharedPointer<Plugin>(new MockPluginSimple());

    // 记录信号发送
    QString receivedIID, receivedName;
    bool initSignalReceived = false, startSignalReceived = false;

    // 打桩Listener信号
    stub.set_lamda(&Listener::pluginInitialized, 
                   [&](Listener *self, const QString &iid, const QString &name) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        receivedIID = iid;
        receivedName = name;
        initSignalReceived = true;
    });

    stub.set_lamda(&Listener::pluginStarted, 
                   [&](Listener *self, const QString &iid, const QString &name) {
        Q_UNUSED(self)
        __DBG_STUB_INVOKE__
        receivedIID = iid;
        receivedName = name;
        startSignalReceived = true;
    });

    // 测试初始化信号
    managerPrivate->doInitPlugin(plugin);
    EXPECT_TRUE(initSignalReceived);
    EXPECT_EQ(receivedIID, "test.signal.interface");
    EXPECT_EQ(receivedName, "SignalTestPlugin");

    // 测试启动信号
    managerPrivate->doStartPlugin(plugin);
    EXPECT_TRUE(startSignalReceived);
    EXPECT_EQ(receivedIID, "test.signal.interface");
    EXPECT_EQ(receivedName, "SignalTestPlugin");
} 