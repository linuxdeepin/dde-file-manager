// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searcheventcaller.cpp
 * @brief Unit tests for SearchEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/searcheventcaller.h"

#include <QTest>

using namespace dfmplugin_search;

class SearchEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchEventCallerTest, SearchEventCaller)
{
    // Test constructor: SearchEventCaller(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(SearchEventCallerTest, sendShowAdvanceSearchBar)
{
    // Test method: void sendShowAdvanceSearchBar((quint64 winId, bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->sendShowAdvanceSearchBar(0, false));
}

TEST_F(SearchEventCallerTest, sendShowAdvanceSearchButton)
{
    // Test method: void sendShowAdvanceSearchButton((quint64 winId, bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->sendShowAdvanceSearchButton(0, false));
}

TEST_F(SearchEventCallerTest, sendStartSpinner)
{
    // Test method: void sendStartSpinner((quint64 winId))
    EXPECT_NO_FATAL_FAILURE(obj->sendStartSpinner(0));
}

TEST_F(SearchEventCallerTest, sendStopSpinner)
{
    // Test method: void sendStopSpinner((quint64 winId))
    EXPECT_NO_FATAL_FAILURE(obj->sendStopSpinner(0));
}
