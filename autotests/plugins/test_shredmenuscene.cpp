// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shredmenuscene.cpp
 * @brief Unit tests for ShredMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "shred/shredmenuscene.h"

#include <QTest>

using namespace dfmplugin_utils;

class ShredMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShredMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShredMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShredMenuSceneTest, ShredMenuScene)
{
    // Test constructor: ShredMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShredMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}
