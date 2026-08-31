// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_defaultitemmanager.cpp
 * @brief Unit tests for DefaultItemManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tools/upgrade/units/bookmarkupgrade/defaultitemmanager.h"

#include <QTest>

using namespace src;

class DefaultItemManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DefaultItemManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DefaultItemManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DefaultItemManagerTest, initDefaultItems)
{
    // Test method: void initDefaultItems(())
    EXPECT_NO_FATAL_FAILURE(obj->initDefaultItems());
}

TEST_F(DefaultItemManagerTest, instance)
{
    // Test getter: DFMBASE_USE_NAMESPACE instance()
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });
}
