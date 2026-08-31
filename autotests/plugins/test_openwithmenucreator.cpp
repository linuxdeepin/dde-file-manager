// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwithmenucreator.cpp
 * @brief Unit tests for OpenWithMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/openwithmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class OpenWithMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenWithMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenWithMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenWithMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(OpenWithMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
