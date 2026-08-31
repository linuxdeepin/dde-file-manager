// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_custommanager.cpp
 * @brief Unit tests for CustomManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/custommanager.h"

#include <QTest>

using namespace dfmplugin_search;

class CustomManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomManagerTest, CustomManager)
{
    // Test constructor: CustomManager(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(CustomManagerTest, instance)
{
    // Test getter: CustomManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(CustomManagerTest, isDisableSearch)
{
    // Test method: bool isDisableSearch((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->isDisableSearch(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CustomManagerTest, isRegisted)
{
    // Test method: bool isRegisted((const QString &scheme))
    QString _arg0{};
    auto result = obj->isRegisted(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CustomManagerTest, isUseNormalMenu)
{
    // Test method: bool isUseNormalMenu((const QString &scheme))
    QString _arg0{};
    auto result = obj->isUseNormalMenu(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CustomManagerTest, redirectedPath)
{
    // Test method: QString redirectedPath((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->redirectedPath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
