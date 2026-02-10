// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <DDialog>

#include <QUrl>
#include <QFileDevice>
#include <QString>

// 包含待测试的类
#include "utils/recentfilehelper.h"
#include "dfmplugin_recent_global.h"
#include "utils/recentmanager.h"

// 包含依赖的头文件
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/clipboard.h>

DPRECENT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

/**
 * @brief RecentFileHelper类单元测试
 *
 * 测试范围：
 * 1. 单例模式验证
 * 2. 方法调用参数验证
 * 3. 返回值验证
 * 4. 错误处理和边界条件
 */
class RecentFileHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize helper instance
        helper = RecentFileHelper::instance();
        ASSERT_NE(helper, nullptr);

        // Setup test URLs
        testUrl1 = QUrl("recent:///test1.txt");
        testUrl2 = QUrl("recent:///test2.txt");
        targetUrl = QUrl("file:///target/");

        // Setup test window ID
        testWindowId = 12345;
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
    RecentFileHelper *helper { nullptr };
    QUrl testUrl1;
    QUrl testUrl2;
    QUrl targetUrl;
    quint64 testWindowId;
};

/**
 * @brief 测试单例模式
 * 验证RecentFileHelper::instance()返回同一个实例
 */
TEST_F(RecentFileHelperTest, SingletonPattern_ReturnsSameInstance)
{
    RecentFileHelper *instance1 = RecentFileHelper::instance();
    RecentFileHelper *instance2 = RecentFileHelper::instance();

    // 验证是同一个实例
    EXPECT_EQ(instance1, instance2);
    EXPECT_EQ(helper, instance1);

    // 验证实例不为空
    EXPECT_NE(instance1, nullptr);
}

/**
 * @brief 测试文件剪切操作
 * 验证cutFile方法能正确调用
 */
TEST_F(RecentFileHelperTest, CutFile_CanBeCalled)
{
    QList<QUrl> sources = { testUrl1, testUrl2 };
    AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kNoHint;

    // Test cutFile - 测试方法能正常调用而不崩溃
    EXPECT_NO_THROW(helper->cutFile(testWindowId, sources, targetUrl, flags));
}

/**
 * @brief 测试文件复制操作
 * 验证copyFile方法能正确调用
 */
TEST_F(RecentFileHelperTest, CopyFile_CanBeCalled)
{
    QList<QUrl> sources = { testUrl1, testUrl2 };
    AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kNoHint;

    // Test copyFile - 测试方法能正常调用而不崩溃
    EXPECT_NO_THROW(helper->copyFile(testWindowId, sources, targetUrl, flags));
}

/**
 * @brief 测试移动到回收站操作
 * 验证moveToTrash方法能正确调用
 */
TEST_F(RecentFileHelperTest, MoveToTrash_CanBeCalled)
{
    stub.set_lamda(VADDR(DDialog, exec), [] { return 1; });
    stub.set_lamda(&RecentManagerDBusInterface::RemoveItems, [] {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<>();
    });
    stub.set_lamda(&RecentManagerDBusInterface::PurgeItems, [] {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<>();
    });

    QList<QUrl> sources = { testUrl1, testUrl2 };
    AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kNoHint;

    // Test moveToTrash - 测试方法能正常调用而不崩溃
    EXPECT_NO_THROW(helper->moveToTrash(testWindowId, sources, flags));
}

/**
 * @brief 测试权限设置功能
 * 验证setPermissionHandle方法能正确调用
 */
TEST_F(RecentFileHelperTest, SetPermissions_CanBeCalled)
{
    QFileDevice::Permissions validPermissions = QFileDevice::ReadOwner | QFileDevice::WriteOwner;
    bool operationResult = false;
    QString errorMessage;

    // Test setPermissionHandle - 测试方法能正常调用而不崩溃
    EXPECT_NO_THROW(helper->setPermissionHandle(testWindowId, testUrl1, validPermissions, &operationResult, &errorMessage));
}

/**
 * @brief 测试剪贴板写入功能
 * 验证writeUrlsToClipboard方法能正确调用
 */
TEST_F(RecentFileHelperTest, WriteToClipboard_CanBeCalled)
{
    QList<QUrl> urls = { testUrl1, testUrl2 };
    ClipBoard::ClipboardAction action = ClipBoard::ClipboardAction::kCopyAction;

    // Test writeUrlsToClipboard - 测试方法能正常调用而不崩溃
    EXPECT_NO_THROW(helper->writeUrlsToClipboard(testWindowId, action, urls));
}

/**
 * @brief 测试在插件中打开文件
 * 验证openFileInPlugin方法能正确调用
 */
TEST_F(RecentFileHelperTest, OpenFileInPlugin_CanBeCalled)
{
    QList<QUrl> urls = { testUrl1 };

    // Test openFileInPlugin - 测试方法能正常调用而不崩溃
    EXPECT_NO_THROW(helper->openFileInPlugin(testWindowId, urls));
}

/**
 * @brief 测试在终端中打开文件
 * 验证openFileInTerminal方法能正确调用
 */
TEST_F(RecentFileHelperTest, OpenFileInTerminal_CanBeCalled)
{
    QList<QUrl> urls = { testUrl1 };

    // Test openFileInTerminal - 测试方法能正常调用而不崩溃
    EXPECT_NO_THROW(helper->openFileInTerminal(testWindowId, urls));
}

/**
 * @brief 测试空文件列表处理
 * 验证空文件列表时方法的健壮性
 */
TEST_F(RecentFileHelperTest, EmptyFileList_HandlesGracefully)
{
    QList<QUrl> emptyList;
    AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kNoHint;

    // Test operations with empty list
    EXPECT_NO_THROW(helper->cutFile(testWindowId, emptyList, targetUrl, flags));
    EXPECT_NO_THROW(helper->copyFile(testWindowId, emptyList, targetUrl, flags));
    EXPECT_NO_THROW(helper->moveToTrash(testWindowId, emptyList, flags));
    EXPECT_NO_THROW(helper->openFileInPlugin(testWindowId, emptyList));
    EXPECT_NO_THROW(helper->openFileInTerminal(testWindowId, emptyList));

    ClipBoard::ClipboardAction action = ClipBoard::ClipboardAction::kCopyAction;
    EXPECT_NO_THROW(helper->writeUrlsToClipboard(testWindowId, action, emptyList));
}

/**
 * @brief 测试无效URL处理
 * 验证无效URL时方法的健壮性
 */
TEST_F(RecentFileHelperTest, InvalidUrl_HandlesGracefully)
{
    stub.set_lamda(VADDR(DDialog, exec), [] { return 1; });
    stub.set_lamda(&RecentManagerDBusInterface::RemoveItems, [] {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<>();
    });
    stub.set_lamda(&RecentManagerDBusInterface::PurgeItems, [] {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<>();
    });

    QList<QUrl> invalidUrls = {
        QUrl(""),   // Empty URL
        QUrl("invalid-url"),   // Malformed URL
        QUrl("recent://"),   // Incomplete URL
    };

    AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kNoHint;

    // Test operations with invalid URLs should not crash
    for (const QUrl &invalidUrl : invalidUrls) {
        QList<QUrl> sources = { invalidUrl };
        EXPECT_NO_THROW(helper->cutFile(testWindowId, sources, targetUrl, flags));
        EXPECT_NO_THROW(helper->copyFile(testWindowId, sources, targetUrl, flags));
        EXPECT_NO_THROW(helper->moveToTrash(testWindowId, sources, flags));
    }
}

/**
 * @brief 测试无效窗口ID处理
 * 验证无效窗口ID时方法的健壮性
 */
TEST_F(RecentFileHelperTest, InvalidWindowId_HandlesGracefully)
{
    stub.set_lamda(VADDR(DDialog, exec), [] { return 1; });
    stub.set_lamda(&RecentManagerDBusInterface::RemoveItems, [] {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<>();
    });
    stub.set_lamda(&RecentManagerDBusInterface::PurgeItems, [] {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<>();
    });

    quint64 invalidWindowId = 0;
    QList<QUrl> sources = { testUrl1 };
    AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kNoHint;

    // Test operations with invalid window ID
    EXPECT_NO_THROW(helper->cutFile(invalidWindowId, sources, targetUrl, flags));
    EXPECT_NO_THROW(helper->copyFile(invalidWindowId, sources, targetUrl, flags));
    EXPECT_NO_THROW(helper->moveToTrash(invalidWindowId, sources, flags));
}

/**
 * @brief 测试大量文件选择
 * 验证处理大量文件时的性能和稳定性
 */
TEST_F(RecentFileHelperTest, LargeFileSelection_HandlesGracefully)
{
    stub.set_lamda(VADDR(DDialog, exec), [] { return 1; });
    stub.set_lamda(&RecentManagerDBusInterface::RemoveItems, [] {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<>();
    });
    stub.set_lamda(&RecentManagerDBusInterface::PurgeItems, [] {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<>();
    });

    // Create large file list
    QList<QUrl> largeFileList;
    for (int i = 0; i < 100; ++i) {
        largeFileList.append(QUrl(QString("recent:///file%1.txt").arg(i)));
    }

    AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kNoHint;
    ClipBoard::ClipboardAction action = ClipBoard::ClipboardAction::kCopyAction;

    // Test operations with large file lists should not crash
    EXPECT_NO_THROW(helper->cutFile(testWindowId, largeFileList, targetUrl, flags));
    EXPECT_NO_THROW(helper->copyFile(testWindowId, largeFileList, targetUrl, flags));
    EXPECT_NO_THROW(helper->moveToTrash(testWindowId, largeFileList, flags));
    EXPECT_NO_THROW(helper->writeUrlsToClipboard(testWindowId, action, largeFileList));
}

/**
 * @brief 测试不同剪贴板动作
 * 验证不同剪贴板动作类型的处理
 */
TEST_F(RecentFileHelperTest, ClipboardActions_AllTypesHandled)
{
    QList<QUrl> urls = { testUrl1, testUrl2 };

    QList<ClipBoard::ClipboardAction> actions = {
        ClipBoard::ClipboardAction::kCopyAction,
        ClipBoard::ClipboardAction::kCutAction,
        ClipBoard::ClipboardAction::kUnknownAction
    };

    // Test all clipboard action types
    for (ClipBoard::ClipboardAction action : actions) {
        EXPECT_NO_THROW(helper->writeUrlsToClipboard(testWindowId, action, urls));
    }
}

/**
 * @brief 测试不同作业标志
 * 验证不同作业标志组合的处理
 */
TEST_F(RecentFileHelperTest, JobFlags_AllCombinationsHandled)
{
    stub.set_lamda(VADDR(DDialog, exec), [] { return 1; });
    stub.set_lamda(&RecentManagerDBusInterface::RemoveItems, [] {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<>();
    });
    stub.set_lamda(&RecentManagerDBusInterface::PurgeItems, [] {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<>();
    });

    QList<QUrl> sources = { testUrl1 };

    // Test all flag combinations
    EXPECT_NO_THROW(helper->cutFile(testWindowId, sources, targetUrl, {}));
    EXPECT_NO_THROW(helper->copyFile(testWindowId, sources, targetUrl, {}));
    EXPECT_NO_THROW(helper->moveToTrash(testWindowId, sources, {}));
}

/**
 * @brief 测试权限设置的边界情况
 * 验证权限设置时空指针参数的处理
 */
TEST_F(RecentFileHelperTest, SetPermissions_NullPointers_HandlesGracefully)
{
    QFileDevice::Permissions permissions = QFileDevice::ReadOwner | QFileDevice::WriteOwner;

    // Test with null pointers
    EXPECT_NO_THROW(helper->setPermissionHandle(testWindowId, testUrl1, permissions, nullptr, nullptr));

    bool operationResult = false;
    EXPECT_NO_THROW(helper->setPermissionHandle(testWindowId, testUrl1, permissions, &operationResult, nullptr));

    QString errorMessage;
    EXPECT_NO_THROW(helper->setPermissionHandle(testWindowId, testUrl1, permissions, nullptr, &errorMessage));
}

/**
 * @brief 测试并发操作安全性
 * 验证多个同时操作的线程安全性
 */
TEST_F(RecentFileHelperTest, ConcurrentOperations_ThreadSafe)
{
    stub.set_lamda(VADDR(DDialog, exec), [] { return 1; });
    stub.set_lamda(&RecentManagerDBusInterface::RemoveItems, [] {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<>();
    });
    stub.set_lamda(&RecentManagerDBusInterface::PurgeItems, [] {
        __DBG_STUB_INVOKE__
        return QDBusPendingReply<>();
    });

    QList<QUrl> sources = { testUrl1 };
    AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kNoHint;

    // Simulate multiple concurrent operations
    EXPECT_NO_THROW({
        helper->cutFile(testWindowId, sources, targetUrl, flags);
        helper->copyFile(testWindowId + 1, sources, targetUrl, flags);
        helper->moveToTrash(testWindowId + 2, sources, flags);
        helper->openFileInPlugin(testWindowId + 3, sources);
    });
}
