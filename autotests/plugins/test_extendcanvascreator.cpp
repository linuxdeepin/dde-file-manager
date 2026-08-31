// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extendcanvascreator.cpp
 * @brief Unit tests for ExtendCanvasCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/extendcanvasscene.h"

#include <QTest>

using namespace ddplugin_organizer;

class ExtendCanvasCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtendCanvasCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtendCanvasCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtendCanvasCreatorTest, create)
{
    // Test getter: DFMBASE_USE_NAMESPACE create()
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });
}

TEST_F(ExtendCanvasCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
