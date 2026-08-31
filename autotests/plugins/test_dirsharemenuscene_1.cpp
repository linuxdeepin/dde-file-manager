// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dirsharemenuscene_1.cpp
 * @brief Unit tests for DirShareMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dirsharemenu/dirsharemenuscene.h"

#include <QTest>

using namespace dfmplugin_dirshare;

class DirShareMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DirShareMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DirShareMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DirShareMenuSceneTest, DirShareMenuScene)
{
    // Test constructor: DirShareMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DirShareMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DirShareMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(DirShareMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DirShareMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

TEST_F(DirShareMenuSceneTest, DirShareMenuScene_Destructor)
{
    // Test method:  ~DirShareMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ DirShareMenuScene *tmp = new DirShareMenuScene(); delete tmp; });
}
