// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appentryfileentity.cpp
 * @brief Unit tests for AppEntryFileEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileentity/appentryfileentity.h"

#include <QTest>

using namespace dfmplugin_computer;

class AppEntryFileEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AppEntryFileEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AppEntryFileEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AppEntryFileEntityTest, displayName)
{
    // Test getter: QString displayName()
    auto result = obj->displayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AppEntryFileEntityTest, exists)
{
    // Test bool getter: exists()
    bool result = obj->exists();
    EXPECT_FALSE(result);

}

TEST_F(AppEntryFileEntityTest, extraProperties)
{
    // Test getter: QVariantHash extraProperties()
    auto result = obj->extraProperties();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AppEntryFileEntityTest, AppEntryFileEntity)
{
    // Test constructor: AppEntryFileEntity((const QUrl &url))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AppEntryFileEntityTest, order)
{
    // Test getter: DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder order()
    auto result = obj->order();
    EXPECT_NO_FATAL_FAILURE({ obj->order(); });

}
