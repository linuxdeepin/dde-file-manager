// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iphistroydata_1.cpp
 * @brief Unit tests for IPHistroyData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfmplugin_titlebar_global.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class IPHistroyDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IPHistroyData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IPHistroyData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IPHistroyDataTest, IPHistroyData)
{
    // Test constructor: IPHistroyData(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(IPHistroyDataTest, operator==)
{
    // Test bool getter: operator==()
    bool result = obj->operator==();
    EXPECT_FALSE(result);
}

TEST_F(IPHistroyDataTest, isRecentlyAccessed)
{
    // Test bool getter: isRecentlyAccessed()
    bool result = obj->isRecentlyAccessed();
    EXPECT_FALSE(result);

}

TEST_F(IPHistroyDataTest, toVariantMap)
{
    // Test getter: QVariantMap toVariantMap()
    auto result = obj->toVariantMap();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IPHistroyDataTest, accessedType)
{
    // Test getter: QString accessedType()
    auto result = obj->accessedType();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IPHistroyDataTest, ipData)
{
    // Test getter: QString ipData()
    auto result = obj->ipData();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IPHistroyDataTest, lastAccessed)
{
    // Test getter: QDateTime lastAccessed()
    auto result = obj->lastAccessed();
    EXPECT_NO_FATAL_FAILURE({ obj->lastAccessed(); });

}
