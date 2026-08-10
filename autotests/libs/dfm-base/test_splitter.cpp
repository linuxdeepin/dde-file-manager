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
