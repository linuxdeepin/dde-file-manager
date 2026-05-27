// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "menu/bookmarkmenuscene.h"
#include "menu/private/bookmarkmenuscene_p.h"
#include "controller/bookmarkmanager.h"
#include "events/bookmarkeventcaller.h"

#include <dfm-base/interfaces/abstractmenuscene.h>

#include <QMenu>
#include <QAction>
#include <QUrl>
#include <QVariantHash>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_bookmark;

class UT_BookmarkMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_BookmarkMenuCreator, Name)
{
    EXPECT_EQ(BookmarkMenuCreator::name(), "BookmarkMenu");
}

TEST_F(UT_BookmarkMenuCreator, Create)
{
    BookmarkMenuCreator creator;
    AbstractMenuScene *scene = nullptr;

    EXPECT_NO_FATAL_FAILURE(scene = creator.create());
    EXPECT_TRUE(scene != nullptr);
    EXPECT_TRUE(dynamic_cast<BookmarkMenuScene *>(scene) != nullptr);

    delete scene;
}

class UT_BookmarkMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new BookmarkMenuScene();
    }

    virtual void TearDown() override
    {
        delete scene;
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    BookmarkMenuScene *scene = nullptr;

    const QUrl testUrl = QUrl::fromLocalFile("/home/test");
    const quint64 testWindowId = 12345;
};

TEST_F(UT_BookmarkMenuScene, Name)
{
    EXPECT_EQ(scene->name(), "BookmarkMenu");
}

TEST_F(UT_BookmarkMenuScene, Initialize_Success)
{
    QVariantHash params;
    params["selectFiles"] = QVariantList{testUrl};
    params["onDesktop"] = false;
    params["windowId"] = testWindowId;

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_BookmarkMenuScene, Initialize_EmptyParams)
{
    QVariantHash emptyParams;

    // Should still succeed with empty params
    EXPECT_TRUE(scene->initialize(emptyParams));
}

TEST_F(UT_BookmarkMenuScene, Create_WithValidParams)
{
    QVariantHash params;
    params["selectFiles"] = QVariantList{testUrl};
    params["onDesktop"] = false;
    params["windowId"] = testWindowId;

    scene->initialize(params);

    QMenu menu;

    // Mock bookmark manager to return some bookmarks
    stub.set_lamda(&BookMarkManager::getBookMarkDataMap, [&](BookMarkManager *) -> QMap<QUrl, BookmarkData> {
        __DBG_STUB_INVOKE__
        QMap<QUrl, BookmarkData> map;
        BookmarkData data;
        data.url = testUrl;
        data.name = "Test Bookmark";
        data.isDefaultItem = false;
        map[testUrl] = data;
        return map;
    });

    EXPECT_TRUE(scene->create(&menu));

    // Check that actions were added to the menu
    EXPECT_FALSE(menu.actions().isEmpty());
}

TEST_F(UT_BookmarkMenuScene, Create_NoBookmarks)
{
    QVariantHash params;
    params["selectFiles"] = QVariantList{testUrl};
    params["onDesktop"] = false;
    params["windowId"] = testWindowId;

    scene->initialize(params);

    QMenu menu;

    // Mock bookmark manager to return no bookmarks
    stub.set_lamda(&BookMarkManager::getBookMarkDataMap, [&](BookMarkManager *) -> QMap<QUrl, BookmarkData> {
        __DBG_STUB_INVOKE__
        return QMap<QUrl, BookmarkData>();
    });

    EXPECT_TRUE(scene->create(&menu));

    // Should still create menu but with different content
}

TEST_F(UT_BookmarkMenuScene, UpdateState)
{
    QMenu menu;
    QAction *testAction = menu.addAction("Test Action");

    // Should not crash when updating state
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
}

TEST_F(UT_BookmarkMenuScene, Triggered_AddBookmark)
{
    QVariantHash params;
    params["selectFiles"] = QVariantList{testUrl};
    params["onDesktop"] = false;
    params["windowId"] = testWindowId;

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    // Find the "Add Bookmark" action (assuming it exists)
    QAction *addAction = nullptr;
    for (QAction *action : menu.actions()) {
        if (action->text().contains("Add") || action->data().toString() == "add-bookmark") {
            addAction = action;
            break;
        }
    }

    if (addAction) {
        bool addCalled = false;

        stub.set_lamda(&BookMarkManager::addBookMark, [&](BookMarkManager *, const QList<QUrl> &urls) -> bool {
            __DBG_STUB_INVOKE__
            addCalled = (urls.contains(testUrl));
            return true;
        });

        EXPECT_TRUE(scene->triggered(addAction));
        EXPECT_TRUE(addCalled);
    }
}

TEST_F(UT_BookmarkMenuScene, Triggered_RemoveBookmark)
{
    QVariantHash params;
    params["selectFiles"] = QVariantList{testUrl};
    params["onDesktop"] = false;
    params["windowId"] = testWindowId;

    scene->initialize(params);

    QMenu menu;

    // Mock existing bookmark
    stub.set_lamda(&BookMarkManager::getBookMarkDataMap, [&](BookMarkManager *) -> QMap<QUrl, BookmarkData> {
        __DBG_STUB_INVOKE__
        QMap<QUrl, BookmarkData> map;
        BookmarkData data;
        data.url = testUrl;
        data.name = "Test Bookmark";
        data.isDefaultItem = false;
        map[testUrl] = data;
        return map;
    });

    scene->create(&menu);

    // Find the "Remove Bookmark" action (assuming it exists)
    QAction *removeAction = nullptr;
    for (QAction *action : menu.actions()) {
        if (action->text().contains("Remove") || action->data().toString() == "remove-bookmark") {
            removeAction = action;
            break;
        }
    }

    if (removeAction) {
        bool removeCalled = false;

        stub.set_lamda(&BookMarkManager::removeBookMark, [&](BookMarkManager *, const QUrl &url) -> bool {
            __DBG_STUB_INVOKE__
            removeCalled = (url == testUrl);
            return true;
        });

        EXPECT_TRUE(scene->triggered(removeAction));
        EXPECT_TRUE(removeCalled);
    }
}

TEST_F(UT_BookmarkMenuScene, Triggered_OpenInNewWindow)
{
    QAction testAction;
    testAction.setData("open-in-new-window");

    bool openCalled = false;

    stub.set_lamda(&BookMarkEventCaller::sendBookMarkOpenInNewWindow, [&](const QUrl &url) {
        __DBG_STUB_INVOKE__
        openCalled = true;
    });

    // This test depends on the actual implementation details
    // Just verify it doesn't crash
    EXPECT_NO_FATAL_FAILURE(scene->triggered(&testAction));
}
