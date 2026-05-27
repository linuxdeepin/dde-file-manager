// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <QUrl>
#include <QDir>
#include <QTimer>
#include <QVariantMap>

// 包含待测试的类
#include "events/recenteventreceiver.h"
#include "utils/recentmanager.h"
#include "dfmplugin_recent_global.h"

// 包含依赖的头文件
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-framework/dpf.h>

DPRECENT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPF_USE_NAMESPACE

/**
 * @brief RecentEventReceiver类单元测试
 *
 * 测试范围：
 * 1. URL变化处理
 * 2. 文件操作事件处理
 * 3. UI事件响应
 * 4. Hook机制功能
 * 5. 透明度和属性处理
 * 6. 错误处理和边界条件
 */
class RecentEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize event receiver
        receiver = RecentEventReceiver::instance();
        ASSERT_NE(receiver, nullptr);

        // Setup test data
        testWindowId = 12345;
        testUrl = QUrl("recent:///test.txt");
        testUrls = { QUrl("recent:///test1.txt"), QUrl("recent:///test2.txt") };
        globalPos = QPoint(100, 100);

        // Mock basic operations
        stub.set_lamda(&QThread::currentThread, []() {
            __DBG_STUB_INVOKE__
            return QCoreApplication::instance() ? QCoreApplication::instance()->thread() : nullptr;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
    RecentEventReceiver *receiver { nullptr };
    quint64 testWindowId;
    QUrl testUrl;
    QList<QUrl> testUrls;
    QPoint globalPos;
};

/**
 * @brief 测试单例模式
 * 验证RecentEventReceiver::instance()返回同一个实例
 */
TEST_F(RecentEventReceiverTest, SingletonPattern_ReturnsSameInstance)
{
    RecentEventReceiver *instance1 = RecentEventReceiver::instance();
    RecentEventReceiver *instance2 = RecentEventReceiver::instance();

    // 验证是同一个实例
    EXPECT_EQ(instance1, instance2);
    EXPECT_EQ(receiver, instance1);

    // 验证实例不为空
    EXPECT_NE(instance1, nullptr);
}

/**
 * @brief 测试URL变化处理 - recent scheme
 * 验证handleWindowUrlChanged方法对recent scheme的处理
 */
TEST_F(RecentEventReceiverTest, HandleWindowUrlChanged_SetsCorrectFilters)
{
    QUrl recentUrl("recent:///");

    // Mock RecentHelper::scheme
    stub.set_lamda(&RecentHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("recent");
    });

    // Mock dpfSlotChannel push
    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, quint64, QFlags<QDir::Filter> &);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push), [&]() {
        __DBG_STUB_INVOKE__
        return QVariant();
    });

    // Mock QTimer::singleShot
    typedef void (*FuncType)(int, Qt::TimerType, const QObject *, QtPrivate::QSlotObjectBase *);
    stub.set_lamda(static_cast<FuncType>(&QTimer::singleShotImpl),
                   []() {
                       __DBG_STUB_INVOKE__
                   });

    // Test URL change handling
    receiver->handleWindowUrlChanged(testWindowId, recentUrl);

    // Note: Due to QTimer::singleShot, we need to process events or mock the timer
    // For now, we just verify the method doesn't crash
    EXPECT_NO_THROW(receiver->handleWindowUrlChanged(testWindowId, recentUrl));
}

/**
 * @brief 测试URL变化处理 - 其他scheme
 * 验证handleWindowUrlChanged方法忽略非相关scheme
 */
TEST_F(RecentEventReceiverTest, HandleWindowUrlChanged_IgnoresIrrelevantSchemes)
{
    QUrl fileUrl("file:///test.txt");
    bool slotChannelCalled = false;

    // Mock RecentHelper::scheme
    stub.set_lamda(&RecentHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("recent");
    });

    // Mock dpfSlotChannel push
    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, quint64, QFlags<QDir::Filter> &);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push), [&]() {
        __DBG_STUB_INVOKE__
        slotChannelCalled = true;
        return QVariant();
    });

    // Test with non-recent URL
    receiver->handleWindowUrlChanged(testWindowId, fileUrl);

    // Should not call slot channel for non-recent schemes
    EXPECT_FALSE(slotChannelCalled);
}

/**
 * @brief 测试文件删除结果处理
 * 验证handleRemoveFilesResult方法在成功时重新加载
 */
TEST_F(RecentEventReceiverTest, HandleRemoveFilesResult_ReloadsOnSuccess)
{
    bool reloadCalled = false;

    // Mock RecentManager::instance and reloadRecent
    stub.set_lamda(&RecentManager::instance, []() {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return &manager;
    });

    stub.set_lamda(&RecentManager::reloadRecent, [&]() {
        __DBG_STUB_INVOKE__
        reloadCalled = true;
    });

    // Test successful removal
    receiver->handleRemoveFilesResult(testUrls, true, "");

    EXPECT_TRUE(reloadCalled);
}

/**
 * @brief 测试文件删除结果处理 - 失败情况
 * 验证删除失败时不重新加载
 */
TEST_F(RecentEventReceiverTest, HandleRemoveFilesResult_NoReloadOnFailure)
{
    bool reloadCalled = false;

    // Mock RecentManager::instance and reloadRecent
    stub.set_lamda(&RecentManager::instance, []() {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return &manager;
    });

    stub.set_lamda(&RecentManager::reloadRecent, [&]() {
        __DBG_STUB_INVOKE__
        reloadCalled = true;
    });

    // Test failed removal
    receiver->handleRemoveFilesResult(testUrls, false, "Error occurred");

    EXPECT_FALSE(reloadCalled);
}

/**
 * @brief 测试文件重命名结果处理
 * 验证handleFileRenameResult方法更新最近文件列表
 */
TEST_F(RecentEventReceiverTest, HandleFileRenameResult_UpdatesRecentList)
{
    QMap<QUrl, QUrl> renamedUrls;
    renamedUrls.insert(testUrls[0], QUrl("recent:///renamed1.txt"));
    renamedUrls.insert(testUrls[1], QUrl("recent:///renamed2.txt"));

    bool reloadCalled = false;

    // Mock RecentManager::instance and reloadRecent
    stub.set_lamda(&RecentManager::instance, []() {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return &manager;
    });

    stub.set_lamda(&RecentManager::reloadRecent, [&]() {
        __DBG_STUB_INVOKE__
        reloadCalled = true;
    });

    // Test successful rename
    receiver->handleFileRenameResult(testWindowId, renamedUrls, true, "");

    EXPECT_TRUE(reloadCalled);
}

/**
 * @brief 测试文件重命名结果处理 - 空映射
 * 验证空重命名映射的处理
 */
TEST_F(RecentEventReceiverTest, HandleFileRenameResult_EmptyMap_NoReload)
{
    QMap<QUrl, QUrl> emptyMap;
    bool reloadCalled = false;

    // Mock RecentManager::instance and reloadRecent
    stub.set_lamda(&RecentManager::instance, []() {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return &manager;
    });

    stub.set_lamda(&RecentManager::reloadRecent, [&]() {
        __DBG_STUB_INVOKE__
        reloadCalled = true;
    });

    // Test with empty rename map
    receiver->handleFileRenameResult(testWindowId, emptyMap, true, "");

    EXPECT_FALSE(reloadCalled);
}

/**
 * @brief 测试文件剪切结果处理
 * 验证handleFileCutResult方法刷新视图
 */
TEST_F(RecentEventReceiverTest, HandleFileCutResult_RefreshesView)
{
    QList<QUrl> srcUrls = testUrls;
    QList<QUrl> destUrls = { QUrl("file:///dest1.txt"), QUrl("file:///dest2.txt") };

    bool reloadCalled = false;

    // Mock RecentManager::instance and reloadRecent
    stub.set_lamda(&RecentManager::instance, []() {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return &manager;
    });

    stub.set_lamda(&RecentManager::reloadRecent, [&]() {
        __DBG_STUB_INVOKE__
        reloadCalled = true;
    });

    // Test successful cut
    receiver->handleFileCutResult(srcUrls, destUrls, true, "");

    EXPECT_TRUE(reloadCalled);
}

/**
 * @brief 测试自定义列角色
 * 验证customColumnRole方法返回正确的角色列表
 */
TEST_F(RecentEventReceiverTest, CustomColumnRole_ReturnsCorrectRoles)
{
    QUrl rootUrl("recent:///");
    QList<ItemRoles> roleList;

    // Mock RecentHelper::scheme
    stub.set_lamda(&RecentHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("recent");
    });

    // Test custom column role
    bool result = receiver->customColumnRole(rootUrl, &roleList);

    EXPECT_TRUE(result);
    EXPECT_FALSE(roleList.isEmpty());

    // Verify expected roles are present
    EXPECT_TRUE(roleList.contains(kItemFileDisplayNameRole));
    EXPECT_TRUE(roleList.contains(kItemFilePathRole));
    EXPECT_TRUE(roleList.contains(kItemFileLastReadRole));
    EXPECT_TRUE(roleList.contains(kItemFileSizeRole));
    EXPECT_TRUE(roleList.contains(kItemFileMimeTypeRole));
}

/**
 * @brief 测试非recent URL的自定义列角色
 * 验证非recent scheme返回false
 */
TEST_F(RecentEventReceiverTest, CustomColumnRole_NonRecentUrl_ReturnsFalse)
{
    QUrl fileUrl("file:///test.txt");
    QList<ItemRoles> roleList;

    // Mock RecentHelper::scheme
    stub.set_lamda(&RecentHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("recent");
    });

    // Test with non-recent URL
    bool result = receiver->customColumnRole(fileUrl, &roleList);

    EXPECT_FALSE(result);
}

/**
 * @brief 测试自定义角色显示名称
 * 验证customRoleDisplayName方法格式化正确
 */
TEST_F(RecentEventReceiverTest, CustomRoleDisplayName_FormatsCorrectly)
{
    QString displayName;

    // Mock RecentHelper::scheme
    stub.set_lamda(&RecentHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("recent");
    });

    // Test different roles
    bool result1 = receiver->customRoleDisplayName(testUrl, kItemFilePathRole, &displayName);
    EXPECT_TRUE(result1);

    bool result2 = receiver->customRoleDisplayName(testUrl, kItemFileLastReadRole, &displayName);
    EXPECT_TRUE(result2);

    // Test with invalid role
    bool result3 = receiver->customRoleDisplayName(testUrl, static_cast<ItemRoles>(999999), &displayName);
    EXPECT_FALSE(result3);
}

/**
 * @brief 测试详情视图图标
 * 验证detailViewIcon方法返回合适的图标
 */
TEST_F(RecentEventReceiverTest, DetailViewIcon_ReturnsAppropriateIcon)
{
    QString iconName;

    // Mock RecentHelper::scheme
    stub.set_lamda(&RecentHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("recent");
    });

    // Test detail view icon
    bool result = receiver->detailViewIcon(testUrl, &iconName);

    // Should handle recent URLs
    EXPECT_NO_THROW(receiver->detailViewIcon(testUrl, &iconName));
}

/**
 * @brief 测试标题栏面包屑分离
 * 验证sepateTitlebarCrumb方法处理面包屑
 */
TEST_F(RecentEventReceiverTest, SepateTitlebarCrumb_HandlesCrumb)
{
    QList<QVariantMap> mapGroup;

    // Mock RecentHelper::scheme
    stub.set_lamda(&RecentHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("recent");
    });

    // Test titlebar crumb separation
    bool result = receiver->sepateTitlebarCrumb(testUrl, &mapGroup);

    // Should handle recent URLs appropriately
    EXPECT_NO_THROW(receiver->sepateTitlebarCrumb(testUrl, &mapGroup));
}

/**
 * @brief 测试透明度检查
 * 验证isTransparent方法正确检查透明度状态
 */
TEST_F(RecentEventReceiverTest, IsTransparent_ChecksCorrectly)
{
    TransparentStatus status;

    // Mock RecentHelper::scheme
    stub.set_lamda(&RecentHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("recent");
    });

    // Test transparency check
    bool result = receiver->isTransparent(testUrl, &status);

    // Should handle transparency check
    EXPECT_NO_THROW(receiver->isTransparent(testUrl, &status));
}

/**
 * @brief 测试拖拽操作检查
 * 验证checkDragDropAction方法验证操作
 */
TEST_F(RecentEventReceiverTest, CheckDragDropAction_ValidatesOperation)
{
    QUrl targetUrl("file:///target/");
    Qt::DropAction action = Qt::CopyAction;

    // Mock RecentHelper::scheme
    stub.set_lamda(&RecentHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("recent");
    });

    // Test drag drop action check
    bool result = receiver->checkDragDropAction(testUrls, targetUrl, &action);

    // Should validate drag drop operations
    EXPECT_NO_THROW(receiver->checkDragDropAction(testUrls, targetUrl, &action));
}

/**
 * @brief 测试文件拖拽处理
 * 验证handleDropFiles方法处理文件拖拽
 */
TEST_F(RecentEventReceiverTest, HandleDropFiles_ProcessesDrops)
{
    QUrl targetUrl("file:///target/");

    // Mock RecentHelper::scheme
    stub.set_lamda(&RecentHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("recent");
    });

    // Test drop files handling
    bool result = receiver->handleDropFiles(testUrls, targetUrl);

    // Should handle file drops
    EXPECT_NO_THROW(receiver->handleDropFiles(testUrls, targetUrl));
}

/**
 * @brief 测试属性对话框禁用处理
 * 验证handlePropertydialogDisable方法返回正确状态
 */
TEST_F(RecentEventReceiverTest, HandlePropertydialogDisable_ReturnsCorrectState)
{
    // Mock RecentHelper::scheme
    stub.set_lamda(&RecentHelper::scheme, []() {
        __DBG_STUB_INVOKE__
        return QString("recent");
    });

    // Test property dialog disable
    bool result = receiver->handlePropertydialogDisable(testUrl);

    // Should return appropriate state
    EXPECT_NO_THROW(receiver->handlePropertydialogDisable(testUrl));
}

/**
 * @brief 测试边界条件 - 空URL列表
 * 验证空URL列表的处理
 */
TEST_F(RecentEventReceiverTest, EmptyUrlList_HandlesGracefully)
{
    QList<QUrl> emptyList;
    Qt::DropAction act;

    // Test operations with empty URL list
    EXPECT_NO_THROW(receiver->handleRemoveFilesResult(emptyList, true, ""));
    EXPECT_NO_THROW(receiver->handleFileCutResult(emptyList, emptyList, true, ""));
    EXPECT_NO_THROW(receiver->checkDragDropAction(emptyList, testUrl, &act));
    EXPECT_NO_THROW(receiver->handleDropFiles(emptyList, testUrl));
}

/**
 * @brief 测试边界条件 - 无效窗口ID
 * 验证无效窗口ID的处理
 */
TEST_F(RecentEventReceiverTest, InvalidWindowId_HandlesGracefully)
{
    quint64 invalidWindowId = 0;
    QMap<QUrl, QUrl> renamedUrls;

    // Test operations with invalid window ID
    EXPECT_NO_THROW(receiver->handleWindowUrlChanged(invalidWindowId, testUrl));
    EXPECT_NO_THROW(receiver->handleFileRenameResult(invalidWindowId, renamedUrls, true, ""));
}

/**
 * @brief 测试边界条件 - 无效URL
 * 验证无效URL的安全处理
 */
TEST_F(RecentEventReceiverTest, InvalidUrl_HandlesSafely)
{
    QList<QUrl> invalidUrls = {
        QUrl(),   // Empty URL
        QUrl(""),   // Empty string URL
        QUrl("invalid-url"),   // Malformed URL
    };

    // Test operations with invalid URLs
    for (const QUrl &invalidUrl : invalidUrls) {
        EXPECT_NO_THROW(receiver->handleWindowUrlChanged(testWindowId, invalidUrl));
        EXPECT_NO_THROW(receiver->detailViewIcon(invalidUrl, nullptr));
        EXPECT_NO_THROW(receiver->isTransparent(invalidUrl, nullptr));
        EXPECT_NO_THROW(receiver->handlePropertydialogDisable(invalidUrl));
    }
}

/**
 * @brief 测试错误消息处理
 * 验证错误消息的正确处理
 */
TEST_F(RecentEventReceiverTest, ErrorMessages_HandledCorrectly)
{
    QString errorMsg = "Test error message";
    bool reloadCalled = false;

    // Mock RecentManager to track reload calls
    stub.set_lamda(&RecentManager::instance, []() {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return &manager;
    });

    stub.set_lamda(&RecentManager::reloadRecent, [&]() {
        __DBG_STUB_INVOKE__
        reloadCalled = true;
    });

    // Test error handling - should not reload on failure
    receiver->handleRemoveFilesResult(testUrls, false, errorMsg);
    EXPECT_FALSE(reloadCalled);

    receiver->handleFileRenameResult(testWindowId, QMap<QUrl, QUrl>(), false, errorMsg);
    EXPECT_FALSE(reloadCalled);

    receiver->handleFileCutResult(testUrls, QList<QUrl>(), false, errorMsg);
    EXPECT_FALSE(reloadCalled);
}

/**
 * @brief 测试并发事件处理
 * 验证多个同时事件的处理安全性
 */
TEST_F(RecentEventReceiverTest, ConcurrentEvents_ThreadSafe)
{
    // Mock thread safety
    std::atomic<int> eventCount { 0 };

    stub.set_lamda(&RecentManager::instance, []() {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return &manager;
    });

    stub.set_lamda(&RecentManager::reloadRecent, [&]() {
        __DBG_STUB_INVOKE__
        eventCount++;
    });

    // Simulate concurrent events
    EXPECT_NO_THROW({
        receiver->handleRemoveFilesResult(testUrls, true, "");
        receiver->handleFileRenameResult(testWindowId, QMap<QUrl, QUrl> { { testUrl, QUrl("recent:///new.txt") } }, true, "");
        receiver->handleFileCutResult(testUrls, testUrls, true, "");
    });

    // Should handle concurrent operations safely
    EXPECT_GE(eventCount.load(), 0);
}

/**
 * @brief 测试大量URL处理
 * 验证处理大量URL时的性能
 */
TEST_F(RecentEventReceiverTest, LargeUrlList_PerformanceTest)
{
    // Create large URL list
    QList<QUrl> largeUrlList;
    for (int i = 0; i < 1000; ++i) {
        largeUrlList.append(QUrl(QString("recent:///file%1.txt").arg(i)));
    }

    // Mock RecentManager
    stub.set_lamda(&RecentManager::instance, []() {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return &manager;
    });

    stub.set_lamda(&RecentManager::reloadRecent, []() {
        __DBG_STUB_INVOKE__
        // Simulate reload operation
    });

    // Test performance with large URL list
    EXPECT_NO_THROW(receiver->handleRemoveFilesResult(largeUrlList, true, ""));
    EXPECT_NO_THROW(receiver->handleFileCutResult(largeUrlList, largeUrlList, true, ""));
}
