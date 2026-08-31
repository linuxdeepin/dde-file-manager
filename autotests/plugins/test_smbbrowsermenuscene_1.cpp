// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbbrowsermenuscene_1.cpp
 * @brief Unit tests for SmbBrowserMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/smbbrowsermenuscene.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbBrowserMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbBrowserMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbBrowserMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbBrowserMenuSceneTest, SmbBrowserMenuScene)
{
    // Test constructor: SmbBrowserMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SmbBrowserMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(SmbBrowserMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SmbBrowserMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(SmbBrowserMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

TEST_F(SmbBrowserMenuSceneTest, SmbBrowserMenuScene_Destructor)
{
    // Test method:  ~SmbBrowserMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ SmbBrowserMenuScene *tmp = new SmbBrowserMenuScene(); delete tmp; });
}
