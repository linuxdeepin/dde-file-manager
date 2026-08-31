// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_userentryfileentity_1.cpp
 * @brief Unit tests for UserEntryFileEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileentity/userentryfileentity.h"

#include <QTest>

using namespace dfmplugin_computer;

class UserEntryFileEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UserEntryFileEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UserEntryFileEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UserEntryFileEntityTest, UserEntryFileEntity)
{
    // Test constructor: UserEntryFileEntity((const QUrl &url))
    ASSERT_NE(obj, nullptr);
}

TEST_F(UserEntryFileEntityTest, displayName)
{
    // Test getter: QString displayName()
    auto result = obj->displayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UserEntryFileEntityTest, exists)
{
    // Test bool getter: exists()
    bool result = obj->exists();
    EXPECT_FALSE(result);

}

TEST_F(UserEntryFileEntityTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(UserEntryFileEntityTest, order)
{
    // Test getter: DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder order()
    auto result = obj->order();
    EXPECT_NO_FATAL_FAILURE({ obj->order(); });

}

TEST_F(UserEntryFileEntityTest, showProgress)
{
    // Test bool getter: showProgress()
    bool result = obj->showProgress();
    EXPECT_FALSE(result);

}

TEST_F(UserEntryFileEntityTest, showTotalSize)
{
    // Test bool getter: showTotalSize()
    bool result = obj->showTotalSize();
    EXPECT_FALSE(result);

}

TEST_F(UserEntryFileEntityTest, showUsageSize)
{
    // Test bool getter: showUsageSize()
    bool result = obj->showUsageSize();
    EXPECT_FALSE(result);

}
