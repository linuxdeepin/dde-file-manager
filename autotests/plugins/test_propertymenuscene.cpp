// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertymenuscene.cpp
 * @brief Unit tests for PropertyMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/propertymenuscene.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PropertyMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PropertyMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PropertyMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PropertyMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(PropertyMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
