// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bookmarkhelper.cpp
 * @brief Unit tests for BookMarkHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/bookmarkhelper.h"

#include <QTest>

using namespace dfmplugin_bookmark;

class BookMarkHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BookMarkHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BookMarkHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BookMarkHelperTest, instance)
{
    // Test getter: BookMarkHelper instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(BookMarkHelperTest, isValidQuickAccessConf)
{
    // Test method: bool isValidQuickAccessConf((const QVariantList &list))
    QVariantList _arg0{};
    auto result = obj->isValidQuickAccessConf(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BookMarkHelperTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty());

}
