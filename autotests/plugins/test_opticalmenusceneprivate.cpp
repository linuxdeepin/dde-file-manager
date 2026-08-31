// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalmenusceneprivate.cpp
 * @brief Unit tests for OpticalMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/opticalmenuscene.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalMenuScenePrivateTest, OpticalMenuScenePrivate)
{
    // Test constructor: OpticalMenuScenePrivate((OpticalMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpticalMenuScenePrivateTest, enablePaste)
{
    // Test bool getter: enablePaste()
    bool result = obj->enablePaste();
    EXPECT_FALSE(result);

}

TEST_F(OpticalMenuScenePrivateTest, findSceneName)
{
    // Test method: QString findSceneName((QAction *act))
    auto result = obj->findSceneName(nullptr);
    EXPECT_TRUE(result.isEmpty());

}
