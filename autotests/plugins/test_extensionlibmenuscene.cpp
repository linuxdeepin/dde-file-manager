// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionlibmenuscene.cpp
 * @brief Unit tests for ExtensionLibMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/menuimpl/extensionlibmenuscene.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionLibMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionLibMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionLibMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionLibMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ExtensionLibMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
