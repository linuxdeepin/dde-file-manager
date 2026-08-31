// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iteratorsearcher_1.cpp
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

TEST_F(IteratorSearcherTest, addResultToMap)
{
    // Test method: void addResultToMap((const QUrl &fileUrl, DFMSearchResultMap &results))
    QUrl _arg0{};
    DFMSearchResultMap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->addResultToMap(_arg0, _arg1));
}

TEST_F(IteratorSearcherTest, addResults)
{
    // Test method: void addResults((const DFMSearchResultMap &newResults))
    DFMSearchResultMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addResults(_arg0));
}

TEST_F(IteratorSearcherTest, hasItem)
{
    // Test bool getter: hasItem()
    bool result = obj->hasItem();
    EXPECT_FALSE(result);

}

TEST_F(IteratorSearcherTest, isSupportSearch)
{
    // Test bool getter: isSupportSearch()
    bool result = obj->isSupportSearch();
    EXPECT_FALSE(result);

}

TEST_F(IteratorSearcherTest, onIteratorCreated)
{
    // Test method: void onIteratorCreated((QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> iterator))
    EXPECT_NO_FATAL_FAILURE(obj->onIteratorCreated(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator>()));
}

TEST_F(IteratorSearcherTest, processDirectory)
{
    // Test method: void processDirectory(())
    EXPECT_NO_FATAL_FAILURE(obj->processDirectory());
}

TEST_F(IteratorSearcherTest, publishBatchedResults)
{
    // Test method: void publishBatchedResults(())
    EXPECT_NO_FATAL_FAILURE(obj->publishBatchedResults());
}

TEST_F(IteratorSearcherTest, requestNextDirectory)
{
    // Test method: void requestNextDirectory(())
    EXPECT_NO_FATAL_FAILURE(obj->requestNextDirectory());
}

TEST_F(IteratorSearcherTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

TEST_F(IteratorSearcherTest, takeAll)
{
    // Test getter: DFMSearchResultMap takeAll()
    auto result = obj->takeAll();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IteratorSearcherTest, takeAllUrls)
{
    // Test getter: QList<QUrl> takeAllUrls()
    auto result = obj->takeAllUrls();
    EXPECT_TRUE(result.isEmpty());

}
