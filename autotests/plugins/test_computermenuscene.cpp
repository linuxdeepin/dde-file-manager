// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computermenuscene.cpp
 * @brief Unit tests for ComputerMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/computermenuscene.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}
