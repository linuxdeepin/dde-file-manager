// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchhistroymanager_1.cpp
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

TEST_F(SearchHistroyManagerTest, SearchHistroyManager)
{
    // Test constructor: SearchHistroyManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SearchHistroyManagerTest, addIPHistoryCache)
{
    // Test method: void addIPHistoryCache((const QString &address))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addIPHistoryCache(_arg0));
}

TEST_F(SearchHistroyManagerTest, getIPHistory)
{
    // Test getter: QList<IPHistroyData> getIPHistory()
    auto result = obj->getIPHistory();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SearchHistroyManagerTest, getSearchHistroy)
{
    // Test getter: QStringList getSearchHistroy()
    auto result = obj->getSearchHistroy();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SearchHistroyManagerTest, handleMountNetworkResult)
{
    // Test method: void handleMountNetworkResult((const QString &address, bool ret, dfmmount::DeviceError err, const QString &))
    QString _arg0{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handleMountNetworkResult(_arg0, false, {}, _arg3));
}

TEST_F(SearchHistroyManagerTest, instance)
{
    // Test getter: SearchHistroyManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(SearchHistroyManagerTest, isValidMount)
{
    // Test method: bool isValidMount((const QString &address, bool ret, dfmmount::DeviceError err))
    QString _arg0{};
    auto result = obj->isValidMount(_arg0, false, {});
    EXPECT_FALSE(result);

}
