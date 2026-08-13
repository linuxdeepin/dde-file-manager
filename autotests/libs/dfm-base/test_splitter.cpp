// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_splitter.cpp
 * @brief Unit tests for Splitter and SplitterHandle (splitter.cpp)
 *
 * Splitter is a QSplitter subclass with a splitPosition property and a
 * custom SplitterHandle that manages cursor override. Constructible in
 * offscreen mode — no display hardware required.
 */

#include <gtest/gtest.h>
#include <dfm-base/widgets/dfmsplitter/splitter.h>

#include <QSplitter>
#include <Qt>
#include <QApplication>
#include <QEvent>
#include <QEnterEvent>

using namespace dfmbase;

TEST(SplitterTest, ConstructHorizontalDoesNotCrash)
{
    Splitter s(Qt::Horizontal);
    EXPECT_EQ(s.orientation(), Qt::Horizontal);
}

TEST(SplitterTest, ConstructVerticalDoesNotCrash)
{
    Splitter s(Qt::Vertical);
    EXPECT_EQ(s.orientation(), Qt::Vertical);
}

TEST(SplitterTest, SplitPositionDefaultsToZero)
{
    Splitter s(Qt::Horizontal);
    EXPECT_EQ(s.splitPosition(), 0);
}

TEST(SplitterTest, SetSplitPositionUpdatesValue)
{
    Splitter s(Qt::Horizontal);
    s.setSplitPosition(100);
    EXPECT_EQ(s.splitPosition(), 100);
}

TEST(SplitterTest, SetSamePositionDoesNotChangeValue)
{
    Splitter s(Qt::Horizontal);
    s.setSplitPosition(50);
    s.setSplitPosition(50);   // no-op: same value
    EXPECT_EQ(s.splitPosition(), 50);
}

TEST(SplitterTest, CreateHandleReturnsSplitterHandle)
{
    Splitter s(Qt::Horizontal);
    QSplitterHandle *handle = s.createHandle();
    ASSERT_NE(handle, nullptr);
    EXPECT_EQ(handle->orientation(), Qt::Horizontal);
    delete handle;
}

// ============================================================
// Additional coverage for SplitterHandle events
// ============================================================

TEST(SplitterTest, SplitterHandle_EnterEvent_Horizontal)
{
    Splitter s(Qt::Horizontal);
    QSplitterHandle *handle = s.createHandle();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QEnterEvent enterEvt(QPointF(0,0), QPointF(0,0), QPointF(0,0));
#else
    QEvent enterEvt(QEvent::Enter);
#endif
    EXPECT_NO_FATAL_FAILURE({ QApplication::sendEvent(handle, &enterEvt); });
    delete handle;
}

TEST(SplitterTest, SplitterHandle_EnterEvent_Vertical)
{
    Splitter s(Qt::Vertical);
    QSplitterHandle *handle = s.createHandle();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QEnterEvent enterEvt(QPointF(0,0), QPointF(0,0), QPointF(0,0));
#else
    QEvent enterEvt(QEvent::Enter);
#endif
    EXPECT_NO_FATAL_FAILURE({ QApplication::sendEvent(handle, &enterEvt); });
    delete handle;
}

TEST(SplitterTest, SplitterHandle_LeaveEvent)
{
    Splitter s(Qt::Horizontal);
    QSplitterHandle *handle = s.createHandle();
    QEvent leaveEvt(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE({ QApplication::sendEvent(handle, &leaveEvt); });
    delete handle;
}

// ============================================================
// Additional coverage for Splitter setSplitPosition edge cases
// ============================================================

TEST(SplitterTest, SetSplitPosition_Negative)
{
    Splitter s(Qt::Horizontal);
    s.setSplitPosition(-10);
    EXPECT_EQ(s.splitPosition(), -10);
}

TEST(SplitterTest, SetSplitPosition_VeryLarge)
{
    Splitter s(Qt::Horizontal);
    s.setSplitPosition(999999);
    EXPECT_EQ(s.splitPosition(), 999999);
}
