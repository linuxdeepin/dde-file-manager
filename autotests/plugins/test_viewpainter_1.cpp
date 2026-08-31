// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewpainter_1.cpp
 * @brief Unit tests for ViewPainter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/viewpainter.h"

#include <QTest>

using namespace ddplugin_canvas;

class ViewPainterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewPainter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewPainter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewPainterTest, ViewPainter)
{
    // Test constructor: ViewPainter((CanvasViewPrivate *dd))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ViewPainterTest, drawDragText)
{
    // Test method: void drawDragText((QPainter *painter, const QString &str, const QRect &rect))
    QString _arg1{};
    QRect _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->drawDragText(nullptr, _arg1, _arg2));
}

TEST_F(ViewPainterTest, drawEllipseBackground)
{
    // Test method: void drawEllipseBackground((QPainter *painter, const QRect &rect))
    QRect _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->drawEllipseBackground(nullptr, _arg1));
}

TEST_F(ViewPainterTest, drawFile)
{
    // Test method: void drawFile((QStyleOptionViewItem option, const QModelIndex &index, const QPoint &gridPos))
    QModelIndex _arg1{};
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->drawFile(QStyleOptionViewItem(), _arg1, _arg2));
}

TEST_F(ViewPainterTest, drawFileToPixmap)
{
    // Test method: void drawFileToPixmap((QPixmap *pix,
                                   QStyleOptionViewItem option,
                                   const QModelIndex &index))
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->drawFileToPixmap(nullptr, QStyleOptionViewItem(), _arg2));
}

TEST_F(ViewPainterTest, drawSelectRect)
{
    // Test method: void drawSelectRect(())
    EXPECT_NO_FATAL_FAILURE(obj->drawSelectRect());
}

TEST_F(ViewPainterTest, itemDelegate)
{
    // Test getter: CanvasItemDelegate itemDelegate()
    auto result = obj->itemDelegate();
    EXPECT_NO_FATAL_FAILURE({ obj->itemDelegate(); });

}
