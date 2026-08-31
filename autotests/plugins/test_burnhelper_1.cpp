// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnhelper_1.cpp
 * @brief Unit tests for BurnHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burnhelper.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnHelperTest, burnIsOnLocalStaging)
{
    // Test method: bool burnIsOnLocalStaging((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->burnIsOnLocalStaging(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BurnHelperTest, defaultBurnFs)
{
    // Test getter: int defaultBurnFs()
    auto result = obj->defaultBurnFs();
    EXPECT_EQ(result, 0);

}
