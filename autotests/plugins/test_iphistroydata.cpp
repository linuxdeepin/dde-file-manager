// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iphistroydata.cpp
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
