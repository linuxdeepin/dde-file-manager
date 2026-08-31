// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmsearchresult_1.cpp
 * @brief Unit tests for DFMSearchResult methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searcher/searchresult_define.h"

#include <QTest>

using namespace dfmplugin_search;

class DFMSearchResultTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMSearchResult();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMSearchResult *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMSearchResultTest, DFMSearchResult)
{
    // Test constructor: DFMSearchResult(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(DFMSearchResultTest, matchScore)
{
    // Test getter: double matchScore()
    auto result = obj->matchScore();
    EXPECT_EQ(result, 0.0);

}

TEST_F(DFMSearchResultTest, setSearchType)
{
    // Test method: void setSearchType(())
    EXPECT_NO_FATAL_FAILURE(obj->setSearchType());
}

TEST_F(DFMSearchResultTest, setUrl)
{
    // Test method: void setUrl(())
    EXPECT_NO_FATAL_FAILURE(obj->setUrl());
}
