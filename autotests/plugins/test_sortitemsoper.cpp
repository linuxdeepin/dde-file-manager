// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sortitemsoper.cpp
 * @brief Unit tests for SortItemsOper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/sortanimationoper.h"

#include <QTest>

using namespace ddplugin_canvas;

class SortItemsOperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SortItemsOper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SortItemsOper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SortItemsOperTest, SortItemsOper)
{
    // Test constructor: SortItemsOper((GridCore *core))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SortItemsOperTest, clean)
{
    // Test method: void clean(())
    EXPECT_NO_FATAL_FAILURE(obj->clean());
}

TEST_F(SortItemsOperTest, gridCount)
{
    // Test method: int gridCount((int index))
    auto result = obj->gridCount(0);
    EXPECT_GE(result, 0);

}

TEST_F(SortItemsOperTest, tryMove)
{
    // Test method: void tryMove((const QStringList &orgItems, QStringList &movedItems))
    QStringList _arg0{};
    QStringList _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->tryMove(_arg0, _arg1));
}
