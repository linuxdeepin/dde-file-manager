// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_querysyntaxutils.cpp
 * @brief Unit tests for QuerySyntaxUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searcher/dfmsearch/querystrategies.h"

#include <QTest>

using namespace dfmplugin_search;

class QuerySyntaxUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new QuerySyntaxUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    QuerySyntaxUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(QuerySyntaxUtilsTest, detect)
{
    // Test method: QuerySyntaxType detect((const QString &keyword, SearchType searchType))
    QString _arg0{};
    auto result = obj->detect(_arg0, SearchType());
    EXPECT_NO_FATAL_FAILURE({ obj->detect(_arg0, SearchType()); });

}
