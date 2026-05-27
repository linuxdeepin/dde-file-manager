// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "events/bookmarkeventreceiver.h"
#include "controller/bookmarkmanager.h"

#include <QUrl>
#include <QMap>

using namespace dfmplugin_bookmark;

class UT_BookMarkEventReceiver : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        receiver = BookMarkEventReceiver::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    BookMarkEventReceiver *receiver = nullptr;

    const QUrl oldUrl = QUrl::fromLocalFile("/home/old");
    const QUrl newUrl = QUrl::fromLocalFile("/home/new");
    const quint64 testWindowId = 12345;
};

TEST_F(UT_BookMarkEventReceiver, Instance)
{
    EXPECT_TRUE(receiver != nullptr);
    EXPECT_EQ(receiver, BookMarkEventReceiver::instance());
}

TEST_F(UT_BookMarkEventReceiver, HandleRenameFile_Success)
{
    bool renameHandled = false;

    stub.set_lamda(&BookMarkManager::fileRenamed, [&](BookMarkManager *, const QUrl &oldUrl, const QUrl &newUrl) {
        __DBG_STUB_INVOKE__
        renameHandled = (oldUrl == this->oldUrl && newUrl == this->newUrl);
    });

    QMap<QUrl, QUrl> renamedUrls;
    renamedUrls[oldUrl] = newUrl;

    EXPECT_NO_FATAL_FAILURE(receiver->handleRenameFile(testWindowId, renamedUrls, true, QString()));
    EXPECT_TRUE(renameHandled);
}

TEST_F(UT_BookMarkEventReceiver, HandleRenameFile_Failed)
{
    bool renameHandled = false;

    stub.set_lamda(&BookMarkManager::fileRenamed, [&](BookMarkManager *, const QUrl &, const QUrl &) {
        __DBG_STUB_INVOKE__
        renameHandled = true;
    });

    QMap<QUrl, QUrl> renamedUrls;
    renamedUrls[oldUrl] = newUrl;

    // When result is false, fileRenamed should not be called
    EXPECT_NO_FATAL_FAILURE(receiver->handleRenameFile(testWindowId, renamedUrls, false, "Error message"));
    EXPECT_FALSE(renameHandled);
}

TEST_F(UT_BookMarkEventReceiver, HandleRenameFile_EmptyMap)
{
    bool renameHandled = false;

    stub.set_lamda(&BookMarkManager::fileRenamed, [&](BookMarkManager *, const QUrl &, const QUrl &) {
        __DBG_STUB_INVOKE__
        renameHandled = true;
    });

    QMap<QUrl, QUrl> emptyUrls;

    EXPECT_NO_FATAL_FAILURE(receiver->handleRenameFile(testWindowId, emptyUrls, true, QString()));
    EXPECT_FALSE(renameHandled);
}

TEST_F(UT_BookMarkEventReceiver, HandleSidebarOrderChanged)
{
    bool orderChangeHandled = false;
    QString testGroup = "QuickAccess";
    QList<QUrl> testUrls = { QUrl::fromLocalFile("/home/test1"), QUrl::fromLocalFile("/home/test2") };

    stub.set_lamda(&BookMarkManager::saveSortedItemsToConfigFile, [&](BookMarkManager *, const QList<QUrl> &urls) {
        __DBG_STUB_INVOKE__
        orderChangeHandled = (urls == testUrls);
    });

    EXPECT_NO_FATAL_FAILURE(receiver->handleSidebarOrderChanged(testWindowId, testGroup, testUrls));
}

TEST_F(UT_BookMarkEventReceiver, HandleSidebarOrderChanged_DifferentGroup)
{
    bool orderChangeHandled = false;
    QString differentGroup = "SomeOtherGroup";
    QList<QUrl> testUrls = { QUrl::fromLocalFile("/home/test1") };

    stub.set_lamda(&BookMarkManager::saveSortedItemsToConfigFile, [&](BookMarkManager *, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        orderChangeHandled = true;
    });

    // Should not handle sidebar order changes for groups other than "QuickAccess"
    EXPECT_NO_FATAL_FAILURE(receiver->handleSidebarOrderChanged(testWindowId, differentGroup, testUrls));
    EXPECT_FALSE(orderChangeHandled);
}

TEST_F(UT_BookMarkEventReceiver, HandleSidebarOrderChanged_EmptyUrls)
{
    bool orderChangeHandled = false;
    QString testGroup = "QuickAccess";
    QList<QUrl> emptyUrls;

    stub.set_lamda(&BookMarkManager::saveSortedItemsToConfigFile, [&](BookMarkManager *, const QList<QUrl> &urls) {
        __DBG_STUB_INVOKE__
        orderChangeHandled = true;
    });

    EXPECT_NO_FATAL_FAILURE(receiver->handleSidebarOrderChanged(testWindowId, testGroup, emptyUrls));
}
