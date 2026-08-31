// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bookmarkmenuscene.cpp
 * @brief Unit tests for BookmarkMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/bookmarkmenuscene.h"

#include <QTest>

using namespace dfmplugin_bookmark;

class BookmarkMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BookmarkMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BookmarkMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BookmarkMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(BookmarkMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BookmarkMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BookmarkMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(BookmarkMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(BookmarkMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

TEST_F(BookmarkMenuSceneTest, BookmarkMenuScene_Destructor)
{
    // Test method:  ~BookmarkMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ BookmarkMenuScene *tmp = new BookmarkMenuScene(); delete tmp; });
}
