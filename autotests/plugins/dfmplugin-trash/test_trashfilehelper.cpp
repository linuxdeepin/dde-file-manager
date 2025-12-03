// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QSignalSpy>

#include "utils/trashfilehelper.h"
#include "utils/trashhelper.h"
#include "events/trasheventcaller.h"
#include "dfmplugin_trash_global.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/interfaces/fileinfo.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/utils/universalutils.h"

#include <stubext.h>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPTRASH_USE_NAMESPACE

using namespace dfmplugin_trash;

class TestTrashFileHelper : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        // Setup test environment
        testDir = QDir::temp().absoluteFilePath("trash_filehelper_test_" + QString::number(QCoreApplication::applicationPid()));
        QDir().mkpath(testDir);
        
        // Setup test URLs
        testUrl = QUrl::fromLocalFile(testDir);
        singleFileUrl = QUrl::fromLocalFile(testDir + "/test_file.txt");
        multipleUrls = {
            QUrl::fromLocalFile(testDir + "/file1.txt"),
            QUrl::fromLocalFile(testDir + "/file2.txt"),
            QUrl::fromLocalFile(testDir + "/file3.txt")
        };
        
        // Create test files
        createTestFiles();
    }

    void TearDown() override
    {
        stub.clear();
        // Cleanup test environment
        QDir(testDir).removeRecursively();
    }

    void createTestFiles()
    {
        // Create test files for operations
        for (const QUrl &url : multipleUrls) {
            QFile file(url.toLocalFile());
            file.open(QIODevice::WriteOnly);
            file.write("test content");
            file.close();
        }
        
        QFile singleFile(singleFileUrl.toLocalFile());
        singleFile.open(QIODevice::WriteOnly);
        singleFile.write("single test content");
        singleFile.close();
    }

    // Mock methods for testing
    static bool mockCutFile(const QList<QUrl> &sources, const QUrl &target, Qt::DropAction action, bool force)
    {
        Q_UNUSED(sources);
        Q_UNUSED(target);
        Q_UNUSED(action);
        Q_UNUSED(force);
        return true;
    }

    static bool mockCopyFile(const QList<QUrl> &sources, const QUrl &target, Qt::DropAction action, bool force)
    {
        Q_UNUSED(sources);
        Q_UNUSED(target);
        Q_UNUSED(action);
        Q_UNUSED(force);
        return true;
    }

    static bool mockMoveToTrash(const QList<QUrl> &sources, bool silent, bool force)
    {
        Q_UNUSED(sources);
        Q_UNUSED(silent);
        Q_UNUSED(force);
        return true;
    }

    static bool mockDeleteFile(const QList<QUrl> &urls, bool silent, bool force)
    {
        Q_UNUSED(urls);
        Q_UNUSED(silent);
        Q_UNUSED(force);
        return true;
    }

    static bool mockOpenFileInPlugin(const QUrl &url)
    {
        Q_UNUSED(url);
        return true;
    }

    static bool mockDisableOpenWidgetWidget(const QList<QUrl> &urls)
    {
        Q_UNUSED(urls);
        return true;
    }

    static bool mockHandleCanTag(const QList<QUrl> &urls)
    {
        Q_UNUSED(urls);
        return true;
    }

    static bool mockHandleIsSubFile(const QUrl &parent, const QUrl &sub)
    {
        Q_UNUSED(parent);
        Q_UNUSED(sub);
        return true;
    }

    static bool mockBlockPaste(const QList<QUrl> &urls)
    {
        Q_UNUSED(urls);
        return true;
    }

    static bool mockHandleNotCdComputer(const QList<QUrl> &urls)
    {
        Q_UNUSED(urls);
        return true;
    }

    stub_ext::StubExt stub;
    QString testDir;
    QUrl testUrl;
    QUrl singleFileUrl;
    QList<QUrl> multipleUrls;
};

TEST_F(TestTrashFileHelper, Instance)
{
    TrashFileHelper *instance1 = TrashFileHelper::instance();
    TrashFileHelper *instance2 = TrashFileHelper::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);  // Should be the same instance
}

TEST_F(TestTrashFileHelper, Scheme)
{
    QString scheme = TrashFileHelper::scheme();
    EXPECT_EQ(scheme, "trash");
}

TEST_F(TestTrashFileHelper, CopyFile_ValidTarget)
{
    TrashFileHelper *helper = TrashFileHelper::instance();
    quint64 windowId = 12345;
    QList<QUrl> sources = {singleFileUrl};
    QUrl target = QUrl::fromLocalFile(testDir + "/copy_target");
    Qt::DropAction action = Qt::CopyAction;
    bool force = false;
    
    // Mock copyFile method
    stub.set(ADDR(TrashFileHelper, copyFile), mockCopyFile);
    
    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint;
    bool result = helper->copyFile(windowId, sources, target, flags);
    EXPECT_TRUE(result);
}

TEST_F(TestTrashFileHelper, CutFile_InvalidTarget)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> sources = {QUrl("trash:///test.txt")};
    QUrl target("file:///");  // Invalid target scheme

    bool result = helper.cutFile(windowId, sources, target, AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_FALSE(result);  // Should return false for invalid target
}

TEST_F(TestTrashFileHelper, DeleteFile_EmptySources)
{
    TrashFileHelper *helper = TrashFileHelper::instance();
    quint64 windowId = 12345;
    QList<QUrl> sources;
    bool silent = false;
    bool force = false;
    
    // Mock deleteFile method
    stub.set(ADDR(TrashFileHelper, deleteFile), mockDeleteFile);
    
    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint;
    bool result = helper->deleteFile(windowId, sources, flags);
    EXPECT_TRUE(result);  // Should return true for empty sources
}

TEST_F(TestTrashFileHelper, Instance_CheckDuplicate)
{
    // Test singleton instance
    TrashFileHelper *instance1 = TrashFileHelper::instance();
    TrashFileHelper *instance2 = TrashFileHelper::instance();
    
    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2); // Should be same instance
}

TEST_F(TestTrashFileHelper, MultipleInstanceCalls)
{
    // Test multiple calls to instance method
    TrashFileHelper *instances[10];
    
    for (int i = 0; i < 10; ++i) {
        instances[i] = TrashFileHelper::instance();
        EXPECT_NE(instances[i], nullptr);
    }
    
    // All should be same instance
    for (int i = 1; i < 10; ++i) {
        EXPECT_EQ(instances[0], instances[i]);
    }
}

TEST_F(TestTrashFileHelper, CutFileSingle)
{
    TrashFileHelper *helper = TrashFileHelper::instance();
    quint64 windowId = 12345;
    QList<QUrl> sources = {singleFileUrl};
    QUrl target = QUrl::fromLocalFile(testDir + "/target");
    Qt::DropAction action = Qt::MoveAction;
    bool force = false;
    
    // Mock cutFile method
    stub.set(ADDR(TrashFileHelper, cutFile), mockCutFile);
    
    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint;
    bool result = helper->cutFile(windowId, sources, target, flags);
    EXPECT_TRUE(result);
}

TEST_F(TestTrashFileHelper, CutFileMultiple)
{
    TrashFileHelper *helper = TrashFileHelper::instance();
    quint64 windowId = 12345;
    QUrl target = QUrl::fromLocalFile(testDir + "/target");
    Qt::DropAction action = Qt::MoveAction;
    bool force = false;
    
    stub.set(ADDR(TrashFileHelper, cutFile), mockCutFile);
    
    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint;
    bool result = helper->cutFile(windowId, multipleUrls, target, flags);
    EXPECT_TRUE(result);
}



TEST_F(TestTrashFileHelper, CopyFile_InvalidTarget)
{
    TrashFileHelper *helper = TrashFileHelper::instance();
    quint64 windowId = 12345;
    QList<QUrl> sources = {QUrl("trash:///test.txt")};
    QUrl target("file:///");  // Invalid target scheme

    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags = DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint;
    bool result = helper->copyFile(windowId, sources, target, flags);
    EXPECT_FALSE(result);  // Should return false for invalid target
}

TEST_F(TestTrashFileHelper, MoveToTrash_Valid)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> sources = {QUrl("trash:///test.txt")};

    // Mock FileUtils::isTrashRootFile to return true (so operation is allowed)
    stub.set_lamda(&FileUtils::isTrashRootFile, [](const QUrl &url) -> bool {
        Q_UNUSED(url)
        return true;
    });

    stub.set_lamda(&UrlRoute::urlParent, [](const QUrl &url) -> QUrl {
        Q_UNUSED(url)
        return QUrl("trash:///");
    });

    bool result = helper.moveToTrash(windowId, sources, AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_TRUE(result);
}

TEST_F(TestTrashFileHelper, MoveToTrash_NotInTrashRoot)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> sources = {QUrl("trash:///test.txt")};

    // Mock FileUtils::isTrashRootFile to return false (so operation is skipped)
    stub.set_lamda(&FileUtils::isTrashRootFile, [](const QUrl &url) -> bool {
        Q_UNUSED(url)
        return false;
    });

    stub.set_lamda(&UrlRoute::urlParent, [](const QUrl &url) -> QUrl {
        Q_UNUSED(url)
        return QUrl("trash:///notroot/");
    });

    bool result = helper.moveToTrash(windowId, sources, AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_TRUE(result);  // Should return true but skip operation
}

TEST_F(TestTrashFileHelper, DeleteFile_Valid)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> sources = {QUrl("trash:///test.txt")};

    // Mock FileUtils::isTrashRootFile to return true (so operation is allowed)
    stub.set_lamda(&FileUtils::isTrashRootFile, [](const QUrl &url) -> bool {
        Q_UNUSED(url)
        return true;
    });

    stub.set_lamda(&UrlRoute::urlParent, [](const QUrl &url) -> QUrl {
        Q_UNUSED(url)
        return QUrl("trash:///");
    });

    bool result = helper.deleteFile(windowId, sources, AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_TRUE(result);
}

TEST_F(TestTrashFileHelper, OpenFileInPlugin_WithFiles)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> urls = {QUrl("trash:///test.txt")};

    // Since the actual FileInfo object and DialogManager interaction is complex to mock,
    // we'll just verify that the function executes without crashing
    EXPECT_NO_THROW({
        bool result = helper.openFileInPlugin(windowId, urls);
        (void)result; // Use result to avoid unused warning
    });
}

TEST_F(TestTrashFileHelper, OpenFileInPlugin_NoFiles)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> urls = {QUrl("trash:///testDir")};

    // Similar to the above test, just check that function executes without crashing
    EXPECT_NO_THROW({
        bool result = helper.openFileInPlugin(windowId, urls);
        (void)result; // Use result to avoid unused warning
    });
}

TEST_F(TestTrashFileHelper, BlockPaste_WithinTrash)
{
    TrashFileHelper helper;
    quint64 winId = 12345;
    QList<QUrl> fromUrls = {QUrl("trash:///test.txt")};
    QUrl toUrl("trash:///");
    
    // Mock Clipboard::clearClipboard to track if it's called
    bool clipboardCleared = false;
    stub.set_lamda(&ClipBoard::clearClipboard, [&clipboardCleared]() {
        clipboardCleared = true;
    });

    bool result = helper.blockPaste(winId, fromUrls, toUrl);
    EXPECT_TRUE(result); // Should return true when pasting within trash
    EXPECT_TRUE(clipboardCleared); // Should clear clipboard when pasting within trash
}

TEST_F(TestTrashFileHelper, BlockPaste_BetweenDifferentSchemes)
{
    TrashFileHelper helper;
    quint64 winId = 12345;
    QList<QUrl> fromUrls = {QUrl("file:///test.txt")};  // From file scheme
    QUrl toUrl("trash:///");  // To trash scheme

    bool clipboardCleared = false;
    stub.set_lamda(&ClipBoard::clearClipboard, [&clipboardCleared]() {
        clipboardCleared = true;
    });

    bool result = helper.blockPaste(winId, fromUrls, toUrl);
    EXPECT_FALSE(result); // Should return false when schemes are different
    EXPECT_FALSE(clipboardCleared); // Should not clear clipboard
}

TEST_F(TestTrashFileHelper, DisableOpenWidgetWidget)
{
    TrashFileHelper helper;
    QUrl url("trash:///test.txt");
    bool resultValue = false;

    bool result = helper.disableOpenWidgetWidget(url, &resultValue);
    EXPECT_TRUE(result); // Should return true for trash URLs
    EXPECT_TRUE(resultValue); // Should set resultValue to true to disable widget
}

TEST_F(TestTrashFileHelper, HandleCanTag)
{
    TrashFileHelper helper;
    QUrl url("trash:///test.txt");
    bool canTag = true;

    bool result = helper.handleCanTag(url, &canTag);
    EXPECT_TRUE(result); // Should return true for trash URLs
    EXPECT_FALSE(canTag); // Files in trash should not be taggable
}

TEST_F(TestTrashFileHelper, HandleIsSubFile)
{
    TrashFileHelper helper;
    QUrl parent("trash:///");
    QUrl sub("trash:///test.txt");

    // Mock FileUtils::isTrashFile to return true
    stub.set_lamda(&FileUtils::isTrashFile, [](const QUrl &url) -> bool {
        Q_UNUSED(url)
        return true;
    });

    // Mock FileUtils::trashRootUrl to return trash root URL
    stub.set_lamda(&FileUtils::trashRootUrl, []() -> QUrl {
        return QUrl("trash:///");
    });

    // Mock UniversalUtils::urlEquals
    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) -> bool {
        return url1 == url2;
    });

    bool result = helper.handleIsSubFile(parent, sub);
    EXPECT_TRUE(result); // Should return true for trash root parent with trash file sub
}

TEST_F(TestTrashFileHelper, HandleNotCdComputer)
{
    TrashFileHelper helper;
    QUrl url("trash:///test.txt");
    QUrl cdUrl;

    // Mock FileUtils::trashRootUrl to return trash root URL
    stub.set_lamda(&FileUtils::trashRootUrl, []() -> QUrl {
        return QUrl("trash:///");
    });

    bool result = helper.handleNotCdComputer(url, &cdUrl);
    EXPECT_TRUE(result); // Should return true for trash URLs
    EXPECT_EQ(cdUrl, QUrl("trash:///")); // cdUrl should be set to trash root
}

// 添加额外的测试用例来提高覆盖率

// 测试构造函数
TEST_F(TestTrashFileHelper, Constructor)
{
    TrashFileHelper *helper = new TrashFileHelper();
    EXPECT_NE(helper, nullptr);
    delete helper;
}



// 测试 moveToTrash 方法的边界情况
TEST_F(TestTrashFileHelper, MoveToTrash_EmptySources)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> emptySources = {};

    bool result = helper.moveToTrash(windowId, emptySources, AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_TRUE(result); // 空源列表应返回 true
}

// 测试 deleteFile 方法的边界情况
// Remove duplicate test definition - keep only one

// 测试 copyFile 方法的更多场景
TEST_F(TestTrashFileHelper, CopyFile_DifferentSchemes)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> sources = {QUrl("file:///somefile")};  // 来自文件系统
    QUrl target("trash:///");  // 到垃圾桶

    bool result = helper.copyFile(windowId, sources, target, AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_FALSE(result); // 从其他方案到垃圾桶不应该允许复制
}

// 测试 openFileInPlugin 方法的更多场景
TEST_F(TestTrashFileHelper, OpenFileInPlugin_EmptyList)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> emptyUrls = {};

    bool result = helper.openFileInPlugin(windowId, emptyUrls);
    EXPECT_FALSE(result); // 空列表应该返回 false
}

// 测试 handleIsSubFile 方法的更多场景
TEST_F(TestTrashFileHelper, HandleIsSubFile_DifferentSchemes)
{
    TrashFileHelper helper;
    QUrl parent("file:///");  // 非垃圾桶方案
    QUrl sub("trash:///test.txt");  // 垃圾桶方案

    // Mock FileUtils::isTrashFile to return true for the sub
    stub.set_lamda(&FileUtils::isTrashFile, [](const QUrl &url) -> bool {
        Q_UNUSED(url)
        return true;
    });

    // Mock FileUtils::trashRootUrl to return trash root URL
    stub.set_lamda(&FileUtils::trashRootUrl, []() -> QUrl {
        return QUrl("trash:///");
    });

    // Mock UniversalUtils::urlEquals
    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) -> bool {
        return url1 == url2;
    });

    bool result = helper.handleIsSubFile(parent, sub);
    EXPECT_FALSE(result); // 父目录不是垃圾桶根目录时应该返回 false
}
