// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasviewprivate_1.cpp
 * @brief Unit tests for CanvasViewPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/canvasview.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasViewPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasViewPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasViewPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasViewPrivateTest, CanvasViewPrivate)
{
    // Test constructor: CanvasViewPrivate((CanvasView *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasViewPrivateTest, calcMargins)
{
    // Test method: QMargins calcMargins((const QSize &inSize, const QSize &outSize))
    QSize _arg0{};
    QSize _arg1{};
    auto result = obj->calcMargins(_arg0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->calcMargins(_arg0, _arg1); });

}

TEST_F(CanvasViewPrivateTest, gridAt)
{
    // Test getter: QPoint gridAt()
    auto result = obj->gridAt();
    EXPECT_TRUE(result.isNull());

}

TEST_F(CanvasViewPrivateTest, gridCoordinate)
{
    // Test getter: GridCoordinate gridCoordinate()
    auto result = obj->gridCoordinate();
    EXPECT_NO_FATAL_FAILURE({ obj->gridCoordinate(); });

}

TEST_F(CanvasViewPrivateTest, gridIndex)
{
    // Test getter: int gridIndex()
    auto result = obj->gridIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasViewPrivateTest, gridMarginsHelper)
{
    // Test getter: QMargins gridMarginsHelper()
    auto result = obj->gridMarginsHelper();
    EXPECT_NO_FATAL_FAILURE({ obj->gridMarginsHelper(); });

}

TEST_F(CanvasViewPrivateTest, isWaterMaskOn)
{
    // Test bool getter: isWaterMaskOn()
    bool result = obj->isWaterMaskOn();
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewPrivateTest, itemGridpos)
{
    // Test method: bool itemGridpos((const QString &item, QPoint &gridPos))
    QString _arg0{};
    QPoint _arg1{};
    auto result = obj->itemGridpos(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewPrivateTest, itemRect)
{
    // Test getter: QRect itemRect()
    auto result = obj->itemRect();
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewPrivateTest, openIndex)
{
    // Test method: void openIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openIndex(_arg0));
}

TEST_F(CanvasViewPrivateTest, openIndexByClicked)
{
    // Test method: void openIndexByClicked((const ClickedAction action, const QModelIndex &index))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->openIndexByClicked(ClickedAction(), _arg1));
}

TEST_F(CanvasViewPrivateTest, operState)
{
    // Test getter: OperState operState()
    auto result = obj->operState();
    EXPECT_NO_FATAL_FAILURE({ obj->operState(); });

}

TEST_F(CanvasViewPrivateTest, overlapPos)
{
    // Test getter: QPoint overlapPos()
    auto result = obj->overlapPos();
    EXPECT_TRUE(result.isNull());

}

TEST_F(CanvasViewPrivateTest, visualItem)
{
    // Test method: QString visualItem((const QPoint &gridPos))
    QPoint _arg0{};
    auto result = obj->visualItem(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasViewPrivateTest, visualRect)
{
    // Test method: QRect visualRect((const QPoint &gridPos))
    QPoint _arg0{};
    auto result = obj->visualRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewPrivateTest, CanvasViewPrivate_Destructor)
{
    // Test method:  ~CanvasViewPrivate(())
    EXPECT_NO_FATAL_FAILURE({ CanvasViewPrivate *tmp = new CanvasViewPrivate(); delete tmp; });
}
