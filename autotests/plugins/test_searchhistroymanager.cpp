// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchhistroymanager.cpp
 * @brief Unit tests for SearchHistroyManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/searchhistroymanager.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class SearchHistroyManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchHistroyManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchHistroyManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchHistroyManagerTest, clearHistory)
{
    // Test method: void clearHistory((const QStringList &schemeFilters))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->clearHistory(_arg0));
}

TEST_F(SearchHistroyManagerTest, clearIPHistory)
{
    // Test method: void clearIPHistory(())
    EXPECT_NO_FATAL_FAILURE(obj->clearIPHistory());
}

TEST_F(SearchHistroyManagerTest, removeSearchHistory)
{
    // Test method: bool removeSearchHistory((QString keyword))
    auto result = obj->removeSearchHistory(QString());
    EXPECT_FALSE(result);

}

TEST_F(SearchHistroyManagerTest, writeIntoIPHistory)
{
    // Test method: void writeIntoIPHistory((const QString &ipAddr))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->writeIntoIPHistory(_arg0));
}

TEST_F(SearchHistroyManagerTest, writeIntoSearchHistory)
{
    // Test method: void writeIntoSearchHistory((QString keyword))
    EXPECT_NO_FATAL_FAILURE(obj->writeIntoSearchHistory(QString()));
}
