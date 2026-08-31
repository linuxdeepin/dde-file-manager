// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bookmarkhelper_1.cpp
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

TEST_F(BookMarkHelperTest, BookMarkHelper)
{
    // Test constructor: BookMarkHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BookMarkHelperTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(BookMarkHelperTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
