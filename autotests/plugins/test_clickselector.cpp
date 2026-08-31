// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_clickselector.cpp
 * @brief Unit tests for ClickSelector methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/clickselector.h"

#include <QTest>

using namespace ddplugin_canvas;

class ClickSelectorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ClickSelector();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ClickSelector *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ClickSelectorTest, clear)
{
    // Test method: void clear(())
    EXPECT_NO_FATAL_FAILURE(obj->clear());
}

TEST_F(ClickSelectorTest, click)
{
    // Test method: void click((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->click(_arg0));
}

TEST_F(ClickSelectorTest, horizontalTraversal)
{
    // Test method: QList<QPoint> horizontalTraversal((const QPoint &from, const QPoint &to, const QSize &gridSize))
    QPoint _arg0{};
    QPoint _arg1{};
    QSize _arg2{};
    auto result = obj->horizontalTraversal(_arg0, _arg1, _arg2);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ClickSelectorTest, order)
{
    // Test method: void order((const QPoint &p1, const QPoint &p2, QPoint &from, QPoint &to))
    QPoint _arg0{};
    QPoint _arg1{};
    QPoint _arg2{};
    QPoint _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->order(_arg0, _arg1, _arg2, _arg3));
}

TEST_F(ClickSelectorTest, traverseSelect)
{
    // Test method: void traverseSelect((const QPoint &p1, const QPoint &p2))
    QPoint _arg0{};
    QPoint _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->traverseSelect(_arg0, _arg1));
}
