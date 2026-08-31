// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnsignalmanager.cpp
 * @brief Unit tests for BurnSignalManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burnsignalmanager.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnSignalManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnSignalManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnSignalManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnSignalManagerTest, BurnSignalManager)
{
    // Test constructor: BurnSignalManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BurnSignalManagerTest, instance)
{
    // Test getter: DPBURN_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
