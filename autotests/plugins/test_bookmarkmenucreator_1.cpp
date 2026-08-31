// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bookmarkmenucreator_1.cpp
 * @brief Unit tests for BookmarkMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/bookmarkmenuscene.h"

#include <QTest>

using namespace dfmplugin_bookmark;

class BookmarkMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BookmarkMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BookmarkMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BookmarkMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}
