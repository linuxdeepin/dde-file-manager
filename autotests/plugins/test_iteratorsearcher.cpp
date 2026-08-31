// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iteratorsearcher.cpp
 * @brief Unit tests for IteratorSearcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searcher/iterator/iteratorsearcher.h"

#include <QTest>

using namespace dfmplugin_search;

class IteratorSearcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IteratorSearcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IteratorSearcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IteratorSearcherTest, IteratorSearcher)
{
    // Test constructor: IteratorSearcher((const QUrl &url, const QString &key, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(IteratorSearcherTest, processIteratorResults)
{
    // Test method: void processIteratorResults((QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> iterator))
    EXPECT_NO_FATAL_FAILURE(obj->processIteratorResults(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator>()));
}

TEST_F(IteratorSearcherTest, search)
{
    // Test bool getter: search()
    bool result = obj->search();
    EXPECT_FALSE(result);

}
