// test_pluginmanager.cpp - PluginManager类单元测试
// 基于原始ut_pluginmanager.cpp，使用新的测试架构

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <QDir>
#include <QTemporaryDir>

// 包含测试框架
#include "../../framework/dfm-test-base.h"

// 包含待测试的类
#include <dfm-framework/lifecycle/pluginmanager.h>
#include <dfm-framework/lifecycle/lifecycle.h>

using namespace dpf;

/**
 * @brief PluginManager类单元测试
 * 
 * 测试范围：
 * 1. 插件管理器基本功能
 * 2. 插件路径管理
 * 3. 插件生命周期管理
 * 4. 插件黑名单管理
 * 5. 插件懒加载管理
 */
class PluginManagerTest : public ::testing::Test
{
protected:
    void SetUp() override {
        // 注意：PluginManager没有公开的instance方法，我们通过LifeCycle访问
        // 这里我们创建一个临时的PluginManager进行测试
        manager = new PluginManager();
        ASSERT_NE(manager, nullptr);
        
        // 创建临时目录用于测试
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());
    }
    
    void TearDown() override {
        // 清理插件管理器状态
        // manager->shutdown();  // 如果有shutdown方法
        
        delete manager;
        delete tempDir;
    }
    
    PluginManager *manager;
    QTemporaryDir *tempDir;
};

/**
 * @brief 测试PluginManager构造函数
 * 验证PluginManager对象能够正确创建
 */
TEST_F(PluginManagerTest, Constructor)
{
    // 验证对象创建成功
    EXPECT_NE(manager, nullptr);
    
    // 验证初始状态
    EXPECT_TRUE(manager->pluginIIDs().isEmpty());
    EXPECT_TRUE(manager->pluginPaths().isEmpty());
    EXPECT_TRUE(manager->blackList().isEmpty());
}

/**
 * @brief 测试单例模式（通过LifeCycle）
 * 验证通过LifeCycle访问的插件管理器
 */
TEST_F(PluginManagerTest, SingletonPattern)
{
    // 通过LifeCycle获取插件路径
    QStringList paths1 = LifeCycle::pluginPaths();
    QStringList paths2 = LifeCycle::pluginPaths();
    
    // 验证返回的是相同的数据（说明是同一个实例）
    EXPECT_EQ(paths1, paths2);
}

/**
 * @brief 测试QObject功能
 * 验证PluginManager继承自QObject的功能
 */
TEST_F(PluginManagerTest, QObjectFunctionality)
{
    // 验证是QObject实例
    QObject *obj = qobject_cast<QObject*>(manager);
    EXPECT_NE(obj, nullptr);
    
    // 验证元对象系统
    const QMetaObject *metaObj = manager->metaObject();
    EXPECT_NE(metaObj, nullptr);
    
    // 验证对象名设置功能
    const QString testName = "TestPluginManager";
    manager->setObjectName(testName);
    EXPECT_EQ(manager->objectName(), testName);
}

/**
 * @brief 测试插件路径管理
 * 验证插件路径的设置和获取
 */
TEST_F(PluginManagerTest, PluginPaths)
{
    QStringList testPaths;
    testPaths << "/test/path1" << "/test/path2" << "/test/path3";
    
    // 设置插件路径
    manager->setPluginPaths(testPaths);
    
    // 验证路径设置成功
    QStringList retrievedPaths = manager->pluginPaths();
    EXPECT_EQ(retrievedPaths, testPaths);
    EXPECT_EQ(retrievedPaths.size(), 3);
    EXPECT_TRUE(retrievedPaths.contains("/test/path1"));
    EXPECT_TRUE(retrievedPaths.contains("/test/path2"));
    EXPECT_TRUE(retrievedPaths.contains("/test/path3"));
}

/**
 * @brief 测试基本功能
 * 验证插件管理器的基本功能
 */
TEST_F(PluginManagerTest, BasicFunctionality)
{
    const QString testPath = "/test/plugins";
    
    // 设置插件路径
    QStringList paths;
    paths << testPath;
    manager->setPluginPaths(paths);
    
    // 验证路径设置
    EXPECT_EQ(manager->pluginPaths().first(), testPath);
    
    // 验证初始状态
    EXPECT_FALSE(manager->isAllPluginsInitialized());
    EXPECT_FALSE(manager->isAllPluginsStarted());
} 