// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchfileinfo_1.cpp
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

TEST_F(SearchFileInfoTest, SearchFileInfo)
{
    // Test constructor: SearchFileInfo((const QUrl &url))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SearchFileInfoTest, displayOf)
{
    // Test method: QString displayOf((const FileInfo::DisplayInfoType type))
    auto result = obj->displayOf(FileInfo::DisplayInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SearchFileInfoTest, nameOf)
{
    // Test method: QString nameOf((const NameInfoType type))
    auto result = obj->nameOf(NameInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SearchFileInfoTest, viewOfTip)
{
    // Test method: QString viewOfTip((const ViewInfoType type))
    auto result = obj->viewOfTip(ViewInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SearchFileInfoTest, SearchFileInfo_Destructor)
{
    // Test method:  ~SearchFileInfo(())
    EXPECT_NO_FATAL_FAILURE({ SearchFileInfo *tmp = new SearchFileInfo(); delete tmp; });
}
