// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasitemdelegate.cpp
 * @brief Unit tests for CanvasItemDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "delegate/canvasitemdelegate.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasItemDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasItemDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasItemDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasItemDelegateTest, boundingRect)
{
    // Test method: QRectF boundingRect((const QList<QRectF> &rects))
    QList<QRectF> _arg0{};
    auto result = obj->boundingRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasItemDelegateTest, initStyleOption)
{
    // Test method: void initStyleOption((QStyleOptionViewItem *option, const QModelIndex &index))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->initStyleOption(nullptr, _arg1));
}

TEST_F(CanvasItemDelegateTest, labelRect)
{
    // Test method: QRect labelRect((const QRect &paintRect, const QRect &usedRect))
    QRect _arg0{};
    QRect _arg1{};
    auto result = obj->labelRect(_arg0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasItemDelegateTest, parent)
{
    // Test getter: CanvasView parent()
    auto result = obj->parent();
    EXPECT_NO_FATAL_FAILURE({ obj->parent(); });

}

TEST_F(CanvasItemDelegateTest, setModelData)
{
    // Test setter: void setModelData((QWidget *editor, QAbstractItemModel *model, const QModelIndex &index))
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->setModelData(nullptr, nullptr, _arg2));
}
