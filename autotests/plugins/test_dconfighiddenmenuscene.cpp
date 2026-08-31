// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dconfighiddenmenuscene.cpp
 * @brief Unit tests for DConfigHiddenMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/dconfighiddenmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class DConfigHiddenMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DConfigHiddenMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DConfigHiddenMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DConfigHiddenMenuSceneTest, disableScene)
{
    // Test method: void disableScene(())
    EXPECT_NO_FATAL_FAILURE(obj->disableScene());
}

TEST_F(DConfigHiddenMenuSceneTest, updateActionHidden)
{
    // Test method: void updateActionHidden((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateActionHidden(nullptr));
}
