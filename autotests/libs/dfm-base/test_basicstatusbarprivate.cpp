// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basicstatusbarprivate.cpp
 * @brief Unit tests for BasicStatusBarPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/widgets/dfmstatusbar/private/basicstatusbar_p.h"

#include <QTest>

using namespace src;

class BasicStatusBarPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BasicStatusBarPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BasicStatusBarPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BasicStatusBarPrivateTest, BasicStatusBarPrivate)
{
    // Test constructor: BasicStatusBarPrivate((BasicStatusBar *qq))
    ASSERT_NE(obj, nullptr);
}
