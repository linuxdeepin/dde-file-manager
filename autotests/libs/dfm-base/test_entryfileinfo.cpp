// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_entryfileinfo.cpp
 * @brief Unit tests for TestEntryEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "libs/dfm-base/test_entryfileinfo.h"

#include <QTest>

using namespace autotests;

class TestEntryEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TestEntryEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TestEntryEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TestEntryEntityTest, TestEntryEntity)
{
    // Test constructor: TestEntryEntity(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(TestEntryEntityTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(TestEntryEntityTest, exists)
{
    // Test bool getter: exists()
    bool result = obj->exists();
    EXPECT_FALSE(result);

}

TEST_F(TestEntryEntityTest, order)
{
    // Test getter: EntryOrder order()
    auto result = obj->order();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(TestEntryEntityTest, displayName)
{
    // Test getter: QString displayName()
    auto result = obj->displayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TestEntryEntityTest, showProgress)
{
    // Test bool getter: showProgress()
    bool result = obj->showProgress();
    EXPECT_FALSE(result);

}

TEST_F(TestEntryEntityTest, showTotalSize)
{
    // Test bool getter: showTotalSize()
    bool result = obj->showTotalSize();
    EXPECT_FALSE(result);

}

TEST_F(TestEntryEntityTest, showUsageSize)
{
    // Test bool getter: showUsageSize()
    bool result = obj->showUsageSize();
    EXPECT_FALSE(result);

}
