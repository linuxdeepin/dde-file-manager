// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computerviewprivate.cpp
 * @brief Unit tests for ComputerViewPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/computerview.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerViewPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerViewPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerViewPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerViewPrivateTest, ComputerViewPrivate)
{
    // Test constructor: ComputerViewPrivate((ComputerView *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComputerViewPrivateTest, visibleItemCount)
{
    // Test getter: int visibleItemCount()
    auto result = obj->visibleItemCount();
    EXPECT_EQ(result, 0);

}
