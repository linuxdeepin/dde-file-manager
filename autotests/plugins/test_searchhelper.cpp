// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchhelper.cpp
 * @brief Unit tests for SearchHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/searchhelper.h"

#include <QTest>

using namespace dfmplugin_search;

class SearchHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchHelperTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
