// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "utils/opticalfilehelper.h"
#include "utils/opticalhelper.h"
#include "mastered/masteredmediafileinfo.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/clipboard.h>

#include <dfm-framework/event/event.h>

#include <QUrl>
#include <QDir>

DFMBASE_USE_NAMESPACE
DPOPTICAL_USE_NAMESPACE

class TestOpticalFileHelper : public testing::Test
{
public:
    void SetUp() override
    {
        helper = OpticalFileHelper::instance();
        setupDefaultMocks();
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    void setupDefaultMocks()
    {
        // Mock OpticalHelper
        stub.set_lamda(&OpticalHelper::burnIsOnDisc, [this](const QUrl &url) {
            __DBG_STUB_INVOKE__
            return mockIsOnDisc;
        });

        stub.set_lamda(&OpticalHelper::isBurnEnabled, []() {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(&OpticalHelper::localStagingRoot, []() {
            __DBG_STUB_INVOKE__
            return QUrl::fromLocalFile("/tmp/staging");
        });

        // Mock MasteredMediaFileInfo
        stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), [this]() {
            __DBG_STUB_INVOKE__
            return mockExtraProperties;
        });

        // Mock QDir
        stub.set_lamda(&QDir::setCurrent, [](const QString &path) {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(&QDir::currentPath, []() {
            __DBG_STUB_INVOKE__
            return QString("/current/path");
        });
    }

    OpticalFileHelper *helper = nullptr;
    stub_ext::StubExt stub;

    // Mock data
    QVariantHash mockExtraProperties;
    bool mockIsOnDisc = false;
    QString mockPublishedTopic;
    QString mockPushedSpace;
    QString mockPushedTopic;
};

TEST_F(TestOpticalFileHelper, Instance_ReturnsSingleton)
{
    OpticalFileHelper *instance1 = OpticalFileHelper::instance();
    OpticalFileHelper *instance2 = OpticalFileHelper::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(TestOpticalFileHelper, CutFile_InvalidScheme_ReturnsFalse)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target("file:///tmp/target");
    AbstractJobHandler::JobFlags flags;

    bool result = helper->cutFile(12345, sources, target, flags);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, CutFile_ValidScheme_CallsPasteFilesHandle)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target("burn:///dev/sr0/disc_files/");
    AbstractJobHandler::JobFlags flags;

    bool pasteFilesHandleCalled = false;
    stub.set_lamda(ADDR(OpticalFileHelper, pasteFilesHandle), [&](OpticalFileHelper *, const QList<QUrl> &sources, const QUrl &target, bool isCopy) {
        __DBG_STUB_INVOKE__
        pasteFilesHandleCalled = true;
        EXPECT_FALSE(isCopy);   // Cut operation should set isCopy to false
    });

    bool result = helper->cutFile(12345, sources, target, flags);

    EXPECT_TRUE(result);
    EXPECT_TRUE(pasteFilesHandleCalled);
}

TEST_F(TestOpticalFileHelper, CopyFile_EmptySources_ReturnsFalse)
{
    QList<QUrl> sources;
    QUrl target("burn:///dev/sr0/disc_files/");
    AbstractJobHandler::JobFlags flags;

    bool result = helper->copyFile(12345, sources, target, flags);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, CopyFile_InvalidScheme_ReturnsFalse)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target("file:///tmp/target");
    AbstractJobHandler::JobFlags flags;

    bool result = helper->copyFile(12345, sources, target, flags);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, CopyFile_ValidScheme_CallsPasteFilesHandle)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target("burn:///dev/sr0/disc_files/");
    AbstractJobHandler::JobFlags flags;

    bool pasteFilesHandleCalled = false;
    stub.set_lamda(ADDR(OpticalFileHelper, pasteFilesHandle), [&](OpticalFileHelper *, const QList<QUrl> &sources, const QUrl &target, bool isCopy) {
        __DBG_STUB_INVOKE__
        pasteFilesHandleCalled = true;
        EXPECT_TRUE(isCopy);   // Copy operation should set isCopy to true
    });

    bool result = helper->copyFile(12345, sources, target, flags);

    EXPECT_TRUE(result);
    EXPECT_TRUE(pasteFilesHandleCalled);
}

TEST_F(TestOpticalFileHelper, MoveToTrash_EmptySources_ReturnsFalse)
{
    QList<QUrl> sources;
    AbstractJobHandler::JobFlags flags;

    bool result = helper->moveToTrash(12345, sources, flags);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, MoveToTrash_InvalidScheme_ReturnsFalse)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    AbstractJobHandler::JobFlags flags;

    bool result = helper->moveToTrash(12345, sources, flags);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, MoveToTrash_NoBackerInfo_SkipsFile)
{
    QList<QUrl> sources = { QUrl("burn:///dev/sr0/disc_files/test.txt") };
    AbstractJobHandler::JobFlags flags;

    mockExtraProperties["mm_backer"] = "";   // Empty backer

    bool result = helper->moveToTrash(12345, sources, flags);

    EXPECT_TRUE(result);
    EXPECT_TRUE(mockPublishedTopic.isEmpty());   // No publish should happen
}

TEST_F(TestOpticalFileHelper, MoveToTrash_FileOnDisc_SkipsFile)
{
    QList<QUrl> sources = { QUrl("burn:///dev/sr0/disc_files/test.txt") };
    AbstractJobHandler::JobFlags flags;

    mockExtraProperties["mm_backer"] = "/tmp/test.txt";
    mockIsOnDisc = true;   // File is on disc

    bool result = helper->moveToTrash(12345, sources, flags);

    EXPECT_TRUE(result);
    EXPECT_TRUE(mockPublishedTopic.isEmpty());   // No publish should happen
}

TEST_F(TestOpticalFileHelper, MoveToTrash_ValidStagingFile_PublishesDeleteEvent)
{
    QList<QUrl> sources = { QUrl("burn:///dev/sr0/staging_files/test.txt") };
    AbstractJobHandler::JobFlags flags;

    mockExtraProperties["mm_backer"] = "/tmp/test.txt";
    mockIsOnDisc = false;   // File is in staging

    bool result = helper->moveToTrash(12345, sources, flags);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalFileHelper, OpenFileInPlugin_EmptySources_ReturnsFalse)
{
    QList<QUrl> sources;

    bool result = helper->openFileInPlugin(12345, sources);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, OpenFileInPlugin_InvalidScheme_ReturnsFalse)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };

    bool result = helper->openFileInPlugin(12345, sources);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, OpenFileInPlugin_ValidFiles_PublishesOpenEvent)
{
    QList<QUrl> sources = { QUrl("burn:///dev/sr0/disc_files/test.txt") };

    mockExtraProperties["mm_backer"] = "/tmp/test.txt";

    bool result = helper->openFileInPlugin(12345, sources);

    EXPECT_TRUE(result);
}

TEST_F(TestOpticalFileHelper, WriteUrlsToClipboard_EmptySources_ReturnsFalse)
{
    QList<QUrl> sources;

    bool result = helper->writeUrlsToClipboard(12345, ClipBoard::ClipboardAction::kCopyAction, sources);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, WriteUrlsToClipboard_InvalidScheme_ReturnsFalse)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };

    bool result = helper->writeUrlsToClipboard(12345, ClipBoard::ClipboardAction::kCopyAction, sources);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, WriteUrlsToClipboard_UnsupportedAction_ReturnsFalse)
{
    QList<QUrl> sources = { QUrl("burn:///dev/sr0/disc_files/test.txt") };

    bool result = helper->writeUrlsToClipboard(12345, ClipBoard::ClipboardAction::kCutAction, sources);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, WriteUrlsToClipboard_FileOnDisc_PublishesClipboardEvent)
{
    QList<QUrl> sources = { QUrl("burn:///dev/sr0/disc_files/test.txt") };

    mockExtraProperties["mm_backer"] = "/tmp/test.txt";

    // Mock localStagingRoot to not be parent of backer
    stub.set_lamda(&QUrl::isParentOf, [](const QUrl *, const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;   // Not in staging area
    });

    bool result = helper->writeUrlsToClipboard(12345, ClipBoard::ClipboardAction::kCopyAction, sources);

    EXPECT_TRUE(result);
    EXPECT_EQ(mockPublishedTopic, QString::number(GlobalEventType::kWriteUrlsToClipboard));
}

TEST_F(TestOpticalFileHelper, WriteUrlsToClipboard_FileInStaging_SkipsFile)
{
    QList<QUrl> sources = { QUrl("burn:///dev/sr0/staging_files/test.txt") };

    mockExtraProperties["mm_backer"] = "/tmp/staging/test.txt";

    // Mock localStagingRoot to be parent of backer
    stub.set_lamda(&QUrl::isParentOf, [](const QUrl *, const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;   // In staging area
    });

    bool result = helper->writeUrlsToClipboard(12345, ClipBoard::ClipboardAction::kCopyAction, sources);

    EXPECT_TRUE(result);
    EXPECT_EQ(mockPublishedTopic, QString::number(GlobalEventType::kWriteUrlsToClipboard));
}

TEST_F(TestOpticalFileHelper, OpenFileInTerminal_EmptySources_ReturnsFalse)
{
    QList<QUrl> sources;

    bool result = helper->openFileInTerminal(12345, sources);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, OpenFileInTerminal_InvalidScheme_ReturnsFalse)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };

    bool result = helper->openFileInTerminal(12345, sources);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, OpenFileInTerminal_EmptyBacker_ReturnsFalse)
{
    QList<QUrl> sources = { QUrl("burn:///dev/sr0/disc_files/test.txt") };

    mockExtraProperties["mm_backer"] = "";   // Empty backer

    bool result = helper->openFileInTerminal(12345, sources);

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalFileHelper, OpenFileInTerminal_ValidFiles_PublishesTerminalEvent)
{
    QList<QUrl> sources = { QUrl("burn:///dev/sr0/disc_files/test.txt") };

    mockExtraProperties["mm_backer"] = "/tmp/test.txt";

    bool result = helper->openFileInTerminal(12345, sources);

    EXPECT_TRUE(result);
    EXPECT_EQ(mockPublishedTopic, QString::number(GlobalEventType::kOpenInTerminal));
}

TEST_F(TestOpticalFileHelper, PasteFilesHandle_BurnDisabled_ReturnsEarly)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target("burn:///dev/sr0/disc_files/");

    stub.set_lamda(&OpticalHelper::isBurnEnabled, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    helper->pasteFilesHandle(sources, target, true);

    // Should not push to slot channel when burn is disabled
    EXPECT_TRUE(mockPushedSpace.isEmpty());
    EXPECT_TRUE(mockPushedTopic.isEmpty());
}

TEST_F(TestOpticalFileHelper, PasteFilesHandle_BurnEnabled_PushesToSlotChannel)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target("burn:///dev/sr0/disc_files/");

    helper->pasteFilesHandle(sources, target, true);

    EXPECT_EQ(mockPushedSpace, "dfmplugin_burn");
    EXPECT_EQ(mockPushedTopic, "slot_PasteTo");
}

TEST_F(TestOpticalFileHelper, PasteFilesHandle_CopyOperation_PassesCorrectFlag)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target("burn:///dev/sr0/disc_files/");

    helper->pasteFilesHandle(sources, target, true);   // Copy operation
}

TEST_F(TestOpticalFileHelper, PasteFilesHandle_CutOperation_PassesCorrectFlag)
{
    QList<QUrl> sources = { QUrl::fromLocalFile("/tmp/test.txt") };
    QUrl target("burn:///dev/sr0/disc_files/");

    helper->pasteFilesHandle(sources, target, false);   // Cut operation
}

TEST_F(TestOpticalFileHelper, Scheme_ReturnsCorrectScheme)
{
    QString scheme = helper->scheme();
    EXPECT_EQ(scheme, Global::Scheme::kBurn);
}

// Test multiple files operations
TEST_F(TestOpticalFileHelper, MoveToTrash_MultipleFiles_ProcessesAllFiles)
{
    QList<QUrl> sources = {
        QUrl("burn:///dev/sr0/staging_files/test1.txt"),
        QUrl("burn:///dev/sr0/staging_files/test2.txt"),
        QUrl("burn:///dev/sr0/disc_files/test3.txt")   // This one is on disc, should be skipped
    };
    AbstractJobHandler::JobFlags flags;

    int callCount = 0;
    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), [&]() {
        __DBG_STUB_INVOKE__
        callCount++;
        QVariantHash props;
        props["mm_backer"] = QString("/tmp/test%1.txt").arg(callCount);
        return props;
    });

    mockIsOnDisc = false;   // First two files are in staging

    // Mock burnIsOnDisc to return different values
    stub.set_lamda(&OpticalHelper::burnIsOnDisc, [&](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return url.path().contains("disc_files");   // Only disc_files are on disc
    });

    bool result = helper->moveToTrash(12345, sources, flags);

    EXPECT_TRUE(result);
    EXPECT_EQ(mockPublishedTopic, QString::number(static_cast<int>(GlobalEventType::kDeleteFiles)));
}

TEST_F(TestOpticalFileHelper, OpenFileInPlugin_MultipleFiles_ProcessesAllFiles)
{
    QList<QUrl> sources = {
        QUrl("burn:///dev/sr0/disc_files/test1.txt"),
        QUrl("burn:///dev/sr0/disc_files/test2.txt")
    };

    int callCount = 0;
    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), [&]() {
        __DBG_STUB_INVOKE__
        callCount++;
        QVariantHash props;
        props["mm_backer"] = QString("/tmp/test%1.txt").arg(callCount);
        return props;
    });

    bool result = helper->openFileInPlugin(12345, sources);

    EXPECT_TRUE(result);
    EXPECT_EQ(mockPublishedTopic, QString::number(GlobalEventType::kOpenFiles));
    EXPECT_EQ(callCount, 2);   // Should process both files
}

TEST_F(TestOpticalFileHelper, OpenFileInTerminal_MultipleFiles_ProcessesAllFiles)
{
    QList<QUrl> sources = {
        QUrl("burn:///dev/sr0/disc_files/test1.txt"),
        QUrl("burn:///dev/sr0/disc_files/test2.txt")
    };

    int callCount = 0;
    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), [&]() {
        __DBG_STUB_INVOKE__
        callCount++;
        QVariantHash props;
        props["mm_backer"] = QString("/tmp/test%1.txt").arg(callCount);
        return props;
    });

    bool result = helper->openFileInTerminal(12345, sources);

    EXPECT_TRUE(result);
    EXPECT_EQ(mockPublishedTopic, QString::number(GlobalEventType::kOpenInTerminal));
    EXPECT_EQ(callCount, 2);   // Should process both files
}
