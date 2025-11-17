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
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
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

TEST_F(TestTrashFileHelper, CutFile_ValidTarget)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> sources = {QUrl("trash:///test.txt")};
    QUrl target("trash:///");
    
    bool result = helper.cutFile(windowId, sources, target, AbstractJobHandler::JobFlag::kNoHint);
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

TEST_F(TestTrashFileHelper, CutFile_EmptySources)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> sources;  // Empty list
    QUrl target("trash:///");

    bool result = helper.cutFile(windowId, sources, target, AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_TRUE(result);  // Should return true for empty sources
}

TEST_F(TestTrashFileHelper, CopyFile_ValidTarget)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> sources = {QUrl("trash:///test.txt")};
    QUrl target("trash:///");
    
    bool result = helper.copyFile(windowId, sources, target, AbstractJobHandler::JobFlag::kNoHint);
    EXPECT_TRUE(result);
}

TEST_F(TestTrashFileHelper, CopyFile_InvalidTarget)
{
    TrashFileHelper helper;
    quint64 windowId = 12345;
    QList<QUrl> sources = {QUrl("trash:///test.txt")};
    QUrl target("file:///");  // Invalid target scheme

    bool result = helper.copyFile(windowId, sources, target, AbstractJobHandler::JobFlag::kNoHint);
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
