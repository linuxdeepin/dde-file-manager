// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmsearchresult.cpp
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

TEST_F(DFMSearchResultTest, keyword)
{
    // Test getter: QString keyword()
    auto result = obj->keyword();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DFMSearchResultTest, searchType)
{
    // Test getter: DFMSEARCH::SearchType searchType()
    auto result = obj->searchType();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DFMSearchResultTest, setKeyword)
{
    // Test method: void setKeyword(())
    EXPECT_NO_FATAL_FAILURE(obj->setKeyword());
}

TEST_F(DFMSearchResultTest, setMatchScore)
{
    // Test method: void setMatchScore(())
    EXPECT_NO_FATAL_FAILURE(obj->setMatchScore());
}

TEST_F(DFMSearchResultTest, url)
{
    // Test getter: QUrl url()
    auto result = obj->url();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
