// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_elidetextlayout.cpp
 * @brief Unit tests for ElideTextLayout (elidetextlayout.cpp)
 */

#include <gtest/gtest.h>
#include <QFont>
#include <QRectF>
#include <QStringList>

#include <dfm-base/utils/elidetextlayout.h>

using namespace dfmbase;

TEST(ElideTextLayoutTest, SetTextAndGetText)
{
    ElideTextLayout layout("hello world");
    EXPECT_EQ(layout.text(), QString("hello world"));
    layout.setText("new text");
    EXPECT_EQ(layout.text(), QString("new text"));
}

TEST(ElideTextLayoutTest, SetAttribute)
{
    ElideTextLayout layout("test");
    QFont font("Arial", 10);
    layout.setAttribute(ElideTextLayout::kFont, font);
    QFont got = layout.attribute<QFont>(ElideTextLayout::kFont);
    EXPECT_EQ(got.family(), font.family());
}

TEST(ElideTextLayoutTest, LayoutReturnsRectListForFittingText)
{
    ElideTextLayout layout("short");
    QRectF rect(0, 0, 200, 50);
    QList<QRectF> result = layout.layout(rect, Qt::ElideNone);
    EXPECT_FALSE(result.isEmpty());
}

TEST(ElideTextLayoutTest, LayoutElideRightTruncates)
{
    ElideTextLayout layout("a very long text that exceeds the width");
    QRectF rect(0, 0, 60, 20);
    QStringList lines;
    layout.layout(rect, Qt::ElideRight, nullptr, Qt::NoBrush, &lines);
    EXPECT_FALSE(lines.isEmpty());
}

TEST(ElideTextLayoutTest, HighlightKeywordsNoCrash)
{
    ElideTextLayout layout("find the keyword here");
    layout.setHighlightKeywords(QStringList { "keyword" });
    layout.setHighlightEnabled(true);
    QRectF rect(0, 0, 200, 50);
    QList<QRectF> result = layout.layout(rect, Qt::ElideNone);
    EXPECT_FALSE(result.isEmpty());
}
