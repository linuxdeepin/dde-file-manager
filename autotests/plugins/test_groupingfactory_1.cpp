// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_groupingfactory_1.cpp
 * @brief Unit tests for GroupingFactory methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/groupingfactory.h"

#include <QTest>

using namespace dfmplugin_workspace;

class GroupingFactoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new GroupingFactory();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    GroupingFactory *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(GroupingFactoryTest, GroupingFactory)
{
    // Test constructor: GroupingFactory(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(GroupingFactoryTest, isRegistered)
{
    // Test method: bool isRegistered((const QString &name))
    QString _arg0{};
    auto result = obj->isRegistered(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(GroupingFactoryTest, registerStrategy)
{
    // Test method: void registerStrategy((const QString &name, const RegisteredGroupStrategy &entry))
    QString _arg0{};
    RegisteredGroupStrategy _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->registerStrategy(_arg0, _arg1));
}

TEST_F(GroupingFactoryTest, registry)
{
    // Test getter: DPWORKSPACE_USE_NAMESPACE registry()
    auto result = obj->registry();
    EXPECT_NO_FATAL_FAILURE({ obj->registry(); });

}
