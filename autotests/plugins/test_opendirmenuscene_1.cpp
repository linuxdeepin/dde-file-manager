// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opendirmenuscene_1.cpp
 * @brief Unit tests for OpenDirMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/opendirmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class OpenDirMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenDirMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenDirMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenDirMenuSceneTest, OpenDirMenuScene)
{
    // Test constructor: OpenDirMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpenDirMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(OpenDirMenuSceneTest, emptyMenu)
{
    // Test method: void emptyMenu((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->emptyMenu(nullptr));
}

TEST_F(OpenDirMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OpenDirMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpenDirMenuSceneTest, normalMenu)
{
    // Test method: void normalMenu((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->normalMenu(nullptr));
}

TEST_F(OpenDirMenuSceneTest, openAsAdminAction)
{
    // Test method: void openAsAdminAction((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->openAsAdminAction(nullptr));
}

TEST_F(OpenDirMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(OpenDirMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}
