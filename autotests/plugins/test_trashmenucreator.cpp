// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashmenucreator.cpp
 * @brief Unit tests for TrashMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/trashmenuscene.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashMenuCreatorTest, create)
{
    // Test getter: DFMBASE_USE_NAMESPACE create()
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });
}

TEST_F(TrashMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
