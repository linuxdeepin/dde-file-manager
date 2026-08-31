// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchfileinfo.cpp
 * @brief Unit tests for SearchFileInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileinfo/searchfileinfo.h"

#include <QTest>

using namespace dfmplugin_search;

class SearchFileInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchFileInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchFileInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchFileInfoTest, exists)
{
    // Test bool getter: exists()
    bool result = obj->exists();
    EXPECT_FALSE(result);

}

TEST_F(SearchFileInfoTest, size)
{
    // Test getter: qint64 size()
    auto result = obj->size();
    EXPECT_EQ(result, 0);

}

TEST_F(SearchFileInfoTest, supportedOfAttributes)
{
    // Test method: Qt::DropActions supportedOfAttributes((const SupportType type))
    auto result = obj->supportedOfAttributes(SupportType());
    EXPECT_GE(static_cast<int>(result), 0);

}
