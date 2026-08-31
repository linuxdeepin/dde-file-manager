// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractsearcher.cpp
 * @brief Unit tests for AbstractSearcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searcher/abstractsearcher.h"

#include <QTest>

using namespace dfmplugin_search;

class AbstractSearcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractSearcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractSearcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractSearcherTest, takeAllUrls)
{
    // Test getter: QList<QUrl> takeAllUrls()
    auto result = obj->takeAllUrls();
    EXPECT_TRUE(result.isEmpty());

}
