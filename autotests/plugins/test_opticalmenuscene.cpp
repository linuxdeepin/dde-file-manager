// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalmenuscene.cpp
 * @brief Unit tests for OpticalMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/opticalmenuscene.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalMenuSceneTest, OpticalMenuScene)
{
    // Test constructor: OpticalMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpticalMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OpticalMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpticalMenuSceneTest, OpticalMenuScene_Destructor)
{
    // Test method:  ~OpticalMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ OpticalMenuScene *tmp = new OpticalMenuScene(); delete tmp; });
}
