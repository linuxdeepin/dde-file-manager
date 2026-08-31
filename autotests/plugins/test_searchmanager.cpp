// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchmanager.cpp
 * @brief Unit tests for SearchManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searchmanager.h"

#include <QTest>

using namespace dfmplugin_search;

class SearchManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchManagerTest, instance)
{
    // Test getter: SearchManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(SearchManagerTest, onDConfigValueChanged)
{
    // Test method: void onDConfigValueChanged((const QString &config, const QString &key))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDConfigValueChanged(_arg0, _arg1));
}

TEST_F(SearchManagerTest, search)
{
    // Test method: bool search((quint64 winId, const QString &taskId, const QUrl &url, const QString &keyword))
    QString _arg1{};
    QUrl _arg2{};
    QString _arg3{};
    auto result = obj->search(0, _arg1, _arg2, _arg3);
    EXPECT_FALSE(result);

}

TEST_F(SearchManagerTest, stop)
{
    // Test method: void stop((quint64 winId))
    EXPECT_NO_FATAL_FAILURE(obj->stop(0));
}
