// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalshareproxy.cpp
 * @brief Unit tests for OpticalShareProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/dbusservice/opticalshareproxy.h"

#include <QTest>

using namespace src;

class OpticalShareProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalShareProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalShareProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalShareProxyTest, clearBurnAttribute)
{
    // Test method: bool clearBurnAttribute((const QString &tag))
    QString _arg0{};
    auto result = obj->clearBurnAttribute(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OpticalShareProxyTest, clearBurnState)
{
    // Test method: bool clearBurnState((const QString &dev))
    QString _arg0{};
    auto result = obj->clearBurnState(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OpticalShareProxyTest, instance)
{
    // Test getter: OpticalShareProxy instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
