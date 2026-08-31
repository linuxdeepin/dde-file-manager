// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmsearcher.cpp
 * @brief Unit tests for DFMSearcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searcher/dfmsearch/dfmsearcher.h"

#include <QTest>

using namespace dfmplugin_search;

class DFMSearcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMSearcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMSearcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMSearcherTest, matchPath)
{
    // Test method: QString matchPath((const QString &path))
    QString _arg0{};
    auto result = obj->matchPath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DFMSearcherTest, realSearchPath)
{
    // Test method: QString realSearchPath((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->realSearchPath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DFMSearcherTest, searchTypeDisplayName)
{
    // Test getter: QString searchTypeDisplayName()
    auto result = obj->searchTypeDisplayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DFMSearcherTest, validateSearchType)
{
    // Test method: bool validateSearchType((const QString &transformedPath, SearchOptions &options))
    QString _arg0{};
    SearchOptions _arg1{};
    auto result = obj->validateSearchType(_arg0, _arg1);
    EXPECT_FALSE(result);

}
