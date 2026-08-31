// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computermenuscene_1.cpp
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

TEST_F(ComputerMenuSceneTest, ComputerMenuScene)
{
    // Test constructor: ComputerMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComputerMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ComputerMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ComputerMenuSceneTest, ComputerMenuScene_Destructor)
{
    // Test method:  ~ComputerMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ ComputerMenuScene *tmp = new ComputerMenuScene(); delete tmp; });
}
