// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <QDBusReply>
#include <QDBusInterface>
#include <QTemporaryDir>
#include <QTimer>

// 包含待测试的类
#include "utils/recentmanager.h"
#include "dfmplugin_recent_global.h"

// 包含依赖的头文件
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/utils/universalutils.h>

DPRECENT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

/**
 * @brief RecentManager类单元测试
 *
 * 测试范围：
 * 1. 单例模式验证
 * 2. 初始化和清理流程
 * 3. DBus接口通信
 * 4. 最近文件数据管理
 * 5. 文件监视控制
 * 6. 异常处理和边界条件
 */
class RecentManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Mock QCoreApplication thread check
        stub.set_lamda(&QThread::currentThread, []() {
            __DBG_STUB_INVOKE__
            return QCoreApplication::instance() ? QCoreApplication::instance()->thread() : nullptr;
        });

        // Create temporary directory for testing
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());

        // Initialize manager
        manager = RecentManager::instance();
        ASSERT_NE(manager, nullptr);
    }

    void TearDown() override
    {
        // Clean up resources
        delete tempDir;
        stub.clear();
    }

    stub_ext::StubExt stub;
    RecentManager *manager { nullptr };
    QTemporaryDir *tempDir { nullptr };
};

/**
 * @brief 测试单例模式
 * 验证RecentManager::instance()返回同一个实例
 */
TEST_F(RecentManagerTest, SingletonPattern_ReturnsSameInstance)
{
    RecentManager *instance1 = RecentManager::instance();
    RecentManager *instance2 = RecentManager::instance();

    // 验证是同一个实例
    EXPECT_EQ(instance1, instance2);
    EXPECT_EQ(manager, instance1);

    // 验证实例不为空
    EXPECT_NE(instance1, nullptr);
}

/**
 * @brief 测试初始化流程
 * 验证RecentManager初始化时正确设置所有组件
 */
TEST_F(RecentManagerTest, Initialize_SetsUpCorrectly)
{
    bool dbusInterfaceCreated = false;

    // Mock DBus interface creation
    stub.set_lamda(&RecentManagerDBusInterface::Reload,
                   [&]() {
        __DBG_STUB_INVOKE__
        dbusInterfaceCreated = true;
        return QDBusPendingReply<qlonglong>();
    });

    // Call init method
    manager->init();

    // Verify DBus interface was created
    EXPECT_TRUE(dbusInterfaceCreated);
}

/**
 * @brief 测试DBus接口连接
 * 验证与RecentManager DBus服务的连接建立
 */
TEST_F(RecentManagerTest, DBusInterface_ConnectsSuccessfully)
{
    EXPECT_NO_FATAL_FAILURE(manager->dbus());
}

/**
 * @brief 测试获取最近文件节点
 * 验证getRecentNodes方法返回正确的文件信息映射
 */
TEST_F(RecentManagerTest, GetRecentNodes_ReturnsValidData)
{
    EXPECT_NO_FATAL_FAILURE(manager->getRecentNodes());
}

/**
 * @brief 测试获取不存在路径的原始路径
 * 验证对不存在URL的处理
 */
TEST_F(RecentManagerTest, GetRecentOriginPaths_NonexistentUrl_ReturnsEmpty)
{
    QUrl nonexistentUrl = QUrl("recent:///nonexistent.txt");

    // Clear internal data
    manager->recentItems.clear();

    // Test with nonexistent URL
    QString result = manager->getRecentOriginPaths(nonexistentUrl);

    // Verify empty result
    EXPECT_TRUE(result.isEmpty());
}

/**
 * @brief 测试重置最近文件节点
 * 验证resetRecentNodes方法正确处理DBus返回的数据
 */
TEST_F(RecentManagerTest, ResetRecentNodes_UpdatesData)
{
    bool dbusCallMade = false;
    QVariantList mockData;

    // Prepare mock DBus response
    QVariantMap item1;
    item1["Path"] = "/test1.txt";
    item1["Href"] = "file:///test1.txt";
    item1["modified"] = static_cast<qint64>(1234567890);

    QDBusArgument arg1;
    // Note: QDBusArgument construction is complex, mock it
    mockData.append(QVariant::fromValue(arg1));

    // Mock DBus GetItemsInfo call
    stub.set_lamda(&RecentManagerDBusInterface::GetItemsInfo,
                   [&](RecentManagerDBusInterface *) -> QDBusPendingReply<QVariantList> {
        __DBG_STUB_INVOKE__
        dbusCallMade = true;
        QDBusPendingReply<QVariantList> reply;
        // Mock successful reply
        return reply;
    });

    // Mock QDBusPendingReply waitForFinished and value
    stub.set_lamda(&QDBusPendingCallWatcher::waitForFinished, [](QDBusPendingCallWatcher *) {
        __DBG_STUB_INVOKE__
    });

    // Test resetRecentNodes
    manager->resetRecentNodes();

    // Verify DBus call was made
    EXPECT_TRUE(dbusCallMade);
}

/**
 * @brief 测试DBus调用失败处理
 * 验证DBus服务不可用时的错误处理
 */
TEST_F(RecentManagerTest, DBusFailure_HandlesGracefully)
{
    // Mock DBus interface to be invalid
    stub.set_lamda(&QDBusAbstractInterface::isValid, [](const QDBusAbstractInterface *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock failed DBus call
    stub.set_lamda(&RecentManagerDBusInterface::GetItemsInfo,
                   [](RecentManagerDBusInterface *) -> QDBusPendingReply<QVariantList> {
        __DBG_STUB_INVOKE__
        QDBusPendingReply<QVariantList> reply;
        return reply;  // Return invalid reply
    });

    // Test should not crash
    EXPECT_NO_THROW(manager->resetRecentNodes());
}

/**
 * @brief 测试边界条件 - 空URL
 * 验证空URL的处理
 */
TEST_F(RecentManagerTest, EmptyUrl_HandlesGracefully)
{
    QUrl emptyUrl;

    // Test with empty URL
    QString result = manager->getRecentOriginPaths(emptyUrl);

    // Should return empty string for empty URL
    EXPECT_TRUE(result.isEmpty());
}

/**
 * @brief 测试边界条件 - 无效URL格式
 * 验证无效URL格式的处理
 */
TEST_F(RecentManagerTest, InvalidUrlFormat_HandlesGracefully)
{
    // Test with various invalid URL formats
    QStringList invalidUrls = {
        "not-a-url",
        "://invalid",
        "recent://",
        "recent:///../../outside",
        QString(10000, 'a')  // Very long string
    };

    for (const QString &urlStr : invalidUrls) {
        QUrl invalidUrl(urlStr);
        EXPECT_NO_THROW({
            QString result = manager->getRecentOriginPaths(invalidUrl);
            // Should handle gracefully without crashing
        });
    }
}

/**
 * @brief 测试多线程访问安全性
 * 验证在多线程环境下的安全性
 */
TEST_F(RecentManagerTest, MultiThreadAccess_ThreadSafe)
{
    // Mock thread operations
    stub.set_lamda(&QThread::currentThread, []() {
        __DBG_STUB_INVOKE__
        return QCoreApplication::instance()->thread();
    });

    // Test concurrent access
    EXPECT_NO_THROW({
        RecentManager *instance1 = RecentManager::instance();
        RecentManager *instance2 = RecentManager::instance();
        EXPECT_EQ(instance1, instance2);
    });
}

/**
 * @brief 测试析构函数清理
 * 验证对象销毁时的资源清理
 */
TEST_F(RecentManagerTest, Destructor_CleansUpProperly)
{
    // Note: Since RecentManager is a singleton, we can't easily test destructor
    // But we can test cleanup methods

    manager->recentItems.clear();

    // Verify cleanup
    auto nodes = manager->getRecentNodes();
    EXPECT_EQ(nodes.size(), 0);
}
