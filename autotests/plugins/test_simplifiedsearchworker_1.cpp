// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_simplifiedsearchworker_1.cpp
 * @brief Unit tests for SimplifiedSearchWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/maincontroller/task/taskcommander.h"

#include <QTest>

using namespace dfmplugin_search;

class SimplifiedSearchWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SimplifiedSearchWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SimplifiedSearchWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SimplifiedSearchWorkerTest, appendSearcher)
{
    // Test method: void appendSearcher((AbstractSearcher *searcher))
    EXPECT_NO_FATAL_FAILURE(obj->appendSearcher(nullptr));
}

TEST_F(SimplifiedSearchWorkerTest, cleanupSearchers)
{
    // Test method: void cleanupSearchers(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanupSearchers());
}

TEST_F(SimplifiedSearchWorkerTest, createSearchersForUrl)
{
    // Test method: void createSearchersForUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createSearchersForUrl(_arg0));
}

TEST_F(SimplifiedSearchWorkerTest, getResultUrls)
{
    // Test getter: QList<QUrl> getResultUrls()
    auto result = obj->getResultUrls();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SimplifiedSearchWorkerTest, getResults)
{
    // Test getter: DFMSearchResultMap getResults()
    auto result = obj->getResults();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SimplifiedSearchWorkerTest, isParentPath)
{
    // Test method: bool isParentPath((const QString &parentPath, const QString &childPath))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->isParentPath(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(SimplifiedSearchWorkerTest, onSearcherFinished)
{
    // Test method: void onSearcherFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onSearcherFinished());
}

TEST_F(SimplifiedSearchWorkerTest, onSearcherUnearthed)
{
    // Test method: void onSearcherUnearthed(())
    EXPECT_NO_FATAL_FAILURE(obj->onSearcherUnearthed());
}

TEST_F(SimplifiedSearchWorkerTest, resolveEnabledSearchTypes)
{
    // Test getter: QList<SearchType> resolveEnabledSearchTypes()
    auto result = obj->resolveEnabledSearchTypes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SimplifiedSearchWorkerTest, startSearch)
{
    // Test method: void startSearch(())
    EXPECT_NO_FATAL_FAILURE(obj->startSearch());
}

TEST_F(SimplifiedSearchWorkerTest, stopSearch)
{
    // Test method: void stopSearch(())
    EXPECT_NO_FATAL_FAILURE(obj->stopSearch());
}

TEST_F(SimplifiedSearchWorkerTest, SimplifiedSearchWorker_Destructor)
{
    // Test method:  ~SimplifiedSearchWorker(())
    EXPECT_NO_FATAL_FAILURE({ SimplifiedSearchWorker *tmp = new SimplifiedSearchWorker(); delete tmp; });
}
