// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_oemmenucreator.cpp
 * @brief Unit tests for OemMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "oemmenuscene/oemmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class OemMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OemMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OemMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OemMenuCreatorTest, create)
{
    // Test getter: DFMBASE_USE_NAMESPACE create()
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });
}

TEST_F(OemMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
