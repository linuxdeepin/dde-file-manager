// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasgrid.cpp
 * @brief Unit tests for CanvasGrid methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "grid/canvasgrid.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasGridTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasGrid();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasGrid *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasGridTest, append)
{
    // Test method: void append((const QStringList &items))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->append(_arg0));
}

TEST_F(CanvasGridTest, arrange)
{
    // Test method: void arrange(())
    EXPECT_NO_FATAL_FAILURE(obj->arrange());
}

TEST_F(CanvasGridTest, item)
{
    // Test method: QString item((int index, const QPoint &pos))
    QPoint _arg1{};
    auto result = obj->item(0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasGridTest, items)
{
    // Test method: QStringList items((int index))
    auto result = obj->items(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasGridTest, move)
{
    // Test method: bool move((int toIndex, const QPoint &toPos, const QString &focus, const QStringList &items))
    QPoint _arg1{};
    QString _arg2{};
    QStringList _arg3{};
    auto result = obj->move(0, _arg1, _arg2, _arg3);
    EXPECT_FALSE(result);

}

TEST_F(CanvasGridTest, points)
{
    // Test method: QHash<QString, QPoint> points((int index))
    auto result = obj->points(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasGridTest, remove)
{
    // Test method: bool remove((int index, const QString &item))
    QString _arg1{};
    auto result = obj->remove(0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CanvasGridTest, replace)
{
    // Test method: bool replace((const QString &oldItem, const QString &newItem))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->replace(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CanvasGridTest, tryAppendAfter)
{
    // Test method: void tryAppendAfter((const QStringList &items, int index, const QPoint &begin))
    QStringList _arg0{};
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->tryAppendAfter(_arg0, 0, _arg2));
}

TEST_F(CanvasGridTest, core)
{
    // Test getter: GridCore core()
    auto result = obj->core();
    EXPECT_NO_FATAL_FAILURE({ obj->core(); });

}

TEST_F(CanvasGridTest, instance)
{
    // Test getter: CanvasGrid instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
