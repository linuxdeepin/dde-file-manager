// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalsharedbus.cpp
 * @brief Unit tests for OpticalShareDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "opticalsharedbus.h"

#include <QTest>

using namespace opticalshare;

class OpticalShareDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalShareDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalShareDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalShareDBusTest, ClearBurnAttribute)
{
    // Test method: bool ClearBurnAttribute((const QString &tag))
    QString _arg0{};
    auto result = obj->ClearBurnAttribute(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OpticalShareDBusTest, ClearBurnState)
{
    // Test method: bool ClearBurnState((const QString &dev))
    QString _arg0{};
    auto result = obj->ClearBurnState(_arg0);
    EXPECT_FALSE(result);

}
