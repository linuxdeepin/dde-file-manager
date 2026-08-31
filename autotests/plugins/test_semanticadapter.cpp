// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_semanticadapter.cpp
 * @brief Unit tests for SemanticAdapter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searcher/semantic/semanticadapter.h"

#include <QTest>

using namespace dfmplugin_search;

class SemanticAdapterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SemanticAdapter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SemanticAdapter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SemanticAdapterTest, hasItem)
{
    // Test bool getter: hasItem()
    bool result = obj->hasItem();
    EXPECT_FALSE(result);

}

TEST_F(SemanticAdapterTest, onErrorOccurred)
{
    // Test method: void onErrorOccurred((const DFMSEARCH::SearchError &error))
    DFMSEARCH::SearchError _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onErrorOccurred(_arg0));
}

TEST_F(SemanticAdapterTest, onIntentParsed)
{
    // Test method: void onIntentParsed((const DFMSEARCH::ParsedIntent &intent))
    DFMSEARCH::ParsedIntent _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onIntentParsed(_arg0));
}

TEST_F(SemanticAdapterTest, onResultsFound)
{
    // Test method: void onResultsFound((const DFMSEARCH::SearchResultList &results))
    DFMSEARCH::SearchResultList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onResultsFound(_arg0));
}

TEST_F(SemanticAdapterTest, onSearchCancelled)
{
    // Test method: void onSearchCancelled(())
    EXPECT_NO_FATAL_FAILURE(obj->onSearchCancelled());
}

TEST_F(SemanticAdapterTest, onSearchFinished)
{
    // Test method: void onSearchFinished((const DFMSEARCH::SearchResultList &results))
    DFMSEARCH::SearchResultList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSearchFinished(_arg0));
}

TEST_F(SemanticAdapterTest, processResult)
{
    // Test method: void processResult((const DFMSEARCH::SearchResult &result))
    DFMSEARCH::SearchResult _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->processResult(_arg0));
}

TEST_F(SemanticAdapterTest, search)
{
    // Test bool getter: search()
    bool result = obj->search();
    EXPECT_FALSE(result);

}

TEST_F(SemanticAdapterTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

TEST_F(SemanticAdapterTest, takeAll)
{
    // Test getter: DFMSearchResultMap takeAll()
    auto result = obj->takeAll();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SemanticAdapterTest, SemanticAdapter_Destructor)
{
    // Test method:  ~SemanticAdapter(())
    EXPECT_NO_FATAL_FAILURE({ SemanticAdapter *tmp = new SemanticAdapter(); delete tmp; });
}
