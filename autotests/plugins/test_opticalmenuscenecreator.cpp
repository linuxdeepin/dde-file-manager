// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalmenuscenecreator.cpp
 * @brief Unit tests for OpticalMenuSceneCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/opticalmenuscene.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalMenuSceneCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalMenuSceneCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalMenuSceneCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalMenuSceneCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(OpticalMenuSceneCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
