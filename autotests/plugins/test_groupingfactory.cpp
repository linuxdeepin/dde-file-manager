// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_groupingfactory.cpp
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

TEST_F(GroupingFactoryTest, registeredStrategies)
{
    // Test method: QList<QPair<QString, QString>> registeredStrategies((const QString &schemeFilter))
    QString _arg0{};
    auto result = obj->registeredStrategies(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
