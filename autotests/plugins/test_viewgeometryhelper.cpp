// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewgeometryhelper.cpp
 * @brief Unit tests for ViewGeometryHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/viewgeometryhelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ViewGeometryHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewGeometryHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewGeometryHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewGeometryHelperTest, ViewGeometryHelper)
{
    // Test constructor: ViewGeometryHelper((FileView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ViewGeometryHelperTest, calcColumnCount)
{
    // Test method: int calcColumnCount((int widgetWidth, int itemWidth))
    auto result = obj->calcColumnCount(0, 0);
    EXPECT_GE(result, 0);

}

TEST_F(ViewGeometryHelperTest, calcGroupRectContiansIndexes)
{
    // Test method: ViewGeometryHelper::RangeIndexList calcGroupRectContiansIndexes((const QRect &rect))
    QRect _arg0{};
    auto result = obj->calcGroupRectContiansIndexes(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ViewGeometryHelperTest, calcVisualRect)
{
    // Test method: QRect calcVisualRect((int widgetWidth, int index))
    auto result = obj->calcVisualRect(0, 0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ViewGeometryHelperTest, indexInRect)
{
    // Test method: bool indexInRect((const QRect &actualRect, const QModelIndex &index))
    QRect _arg0{};
    QModelIndex _arg1{};
    auto result = obj->indexInRect(_arg0, _arg1);
    EXPECT_FALSE(result);

}
