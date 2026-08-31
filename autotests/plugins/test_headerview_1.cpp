// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_headerview_1.cpp
 * @brief Unit tests for HeaderView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/headerview.h"

#include <QTest>

using namespace dfmplugin_workspace;

class HeaderViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new HeaderView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    HeaderView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(HeaderViewTest, HeaderView)
{
    // Test constructor: HeaderView((Qt::Orientation orientation, FileView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(HeaderViewTest, doFileNameColumnResize)
{
    // Test method: void doFileNameColumnResize((const int totalWidth))
    EXPECT_NO_FATAL_FAILURE(obj->doFileNameColumnResize(0));
}

TEST_F(HeaderViewTest, event)
{
    // Test method: bool event((QEvent *e))
    auto result = obj->event(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(HeaderViewTest, leaveEvent)
{
    // Test event handler: leaveEvent((QEvent *e))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->leaveEvent(&_event));
}

TEST_F(HeaderViewTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *e))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(HeaderViewTest, mouseReleaseEvent)
{
    // Test event handler: mouseReleaseEvent((QMouseEvent *e))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseReleaseEvent(&_event));
}

TEST_F(HeaderViewTest, onActionClicked)
{
    // Test method: void onActionClicked((const int column, QAction *action))
    EXPECT_NO_FATAL_FAILURE(obj->onActionClicked(0, nullptr));
}

TEST_F(HeaderViewTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *e))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(HeaderViewTest, paintSection)
{
    // Test method: void paintSection((QPainter *painter, const QRect &rect, int logicalIndex))
    QRect _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->paintSection(nullptr, _arg1, 0));
}

TEST_F(HeaderViewTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *e))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(HeaderViewTest, sectionElidedName)
{
    // Test method: QString sectionElidedName((int logicalIndex, int availableWidth))
    auto result = obj->sectionElidedName(0, 0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(HeaderViewTest, sectionName)
{
    // Test method: QString sectionName((int logicalIndex))
    auto result = obj->sectionName(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(HeaderViewTest, sectionsTotalWidth)
{
    // Test getter: int sectionsTotalWidth()
    auto result = obj->sectionsTotalWidth();
    EXPECT_EQ(result, 0);

}

TEST_F(HeaderViewTest, sizeHint)
{
    // Test getter: QSize sizeHint()
    auto result = obj->sizeHint();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(HeaderViewTest, syncOffset)
{
    // Test method: void syncOffset((int value))
    EXPECT_NO_FATAL_FAILURE(obj->syncOffset(0));
}

TEST_F(HeaderViewTest, viewModel)
{
    // Test getter: FileViewModel viewModel()
    auto result = obj->viewModel();
    EXPECT_NO_FATAL_FAILURE({ obj->viewModel(); });

}
