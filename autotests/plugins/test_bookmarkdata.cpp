// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bookmarkdata.cpp
 * @brief Unit tests for BookmarkData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tools/upgrade/units/bookmarkupgradeunit.h"

#include <QTest>

using namespace src;

class BookmarkDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BookmarkData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BookmarkData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BookmarkDataTest, serialize)
{
    // Test getter: QVariantMap serialize()
    auto result = obj->serialize();
    EXPECT_TRUE(result.isEmpty());

}
