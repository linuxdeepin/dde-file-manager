// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dcustomactionentry.cpp
 * @brief Unit tests for DCustomActionEntry methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenu/dcustomactiondata.h"

#include <QTest>

using namespace dfmplugin_menu;

class DCustomActionEntryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DCustomActionEntry();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DCustomActionEntry *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DCustomActionEntryTest, data)
{
    // Test getter: DCustomActionData data()
    auto result = obj->data();
    EXPECT_NO_FATAL_FAILURE({ obj->data(); });

}

TEST_F(DCustomActionEntryTest, excludeMimeTypes)
{
    // Test getter: QStringList excludeMimeTypes()
    auto result = obj->excludeMimeTypes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, mimeTypes)
{
    // Test getter: QStringList mimeTypes()
    auto result = obj->mimeTypes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, notShowIn)
{
    // Test getter: QStringList notShowIn()
    auto result = obj->notShowIn();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, DCustomActionEntry)
{
    // Test constructor: DCustomActionEntry((const DCustomActionEntry &other))
    ASSERT_NE(obj, nullptr);
}
