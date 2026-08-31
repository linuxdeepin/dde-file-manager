// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extendmenuscene.cpp
 * @brief Unit tests for ExtendMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ExtendMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtendMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtendMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtendMenuSceneTest, ExtendMenuScene)
{
    // Test constructor: ExtendMenuScene((DCustomActionParser *parser, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ExtendMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ExtendMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
