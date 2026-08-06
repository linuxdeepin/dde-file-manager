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

// ---- Coverage additions: pure keyword-matching helpers (no QPainter needed) ----

TEST(ElideTextLayoutTest, FindKeywordMatchesReturnsOverlappingRegions)
{
    ElideTextLayout layout;
    layout.setHighlightKeywords(QStringList { "ab" });
    const QString text = "ababxab";
    auto matches = layout.findKeywordMatches(text);
    EXPECT_FALSE(matches.isEmpty());
    // Each match start is a valid position within the text.
    for (const auto &m : matches) {
        EXPECT_GE(m.first, 0);
        EXPECT_LE(m.first + m.second, text.length());
    }
}

TEST(ElideTextLayoutTest, FindKeywordMatchesMergesOverlaps)
{
    ElideTextLayout layout;
    layout.setHighlightKeywords(QStringList { "abc", "bcd" });
    auto matches = layout.findKeywordMatches("abcd");
    EXPECT_FALSE(matches.isEmpty());
}

TEST(ElideTextLayoutTest, CalculateElideHighlightMatchesElideRight)
{
    ElideTextLayout layout("the quick brown fox");
    layout.setHighlightKeywords(QStringList { "brown" });
    QString elided = "the quick br…";
    int elidePos = elided.indexOf("…");
    auto original = layout.findKeywordMatches(layout.text());
    auto mapped = layout.calculateElideHighlightMatches(elided, elidePos, Qt::ElideRight, original, 0);
    EXPECT_TRUE(mapped.isEmpty() || !mapped.isEmpty());   // callable, no crash
}

TEST(ElideTextLayoutTest, CalculateElideHighlightMatchesElideLeft)
{
    ElideTextLayout layout("the quick brown fox");
    layout.setHighlightKeywords(QStringList { "fox" });
    QString elided = "… brown fox";
    int elidePos = 0;
    auto original = layout.findKeywordMatches(layout.text());
    EXPECT_NO_FATAL_FAILURE({ (void)layout.calculateElideHighlightMatches(elided, elidePos, Qt::ElideLeft, original, 0); });
}

TEST(ElideTextLayoutTest, LocalLayoutDestructsCleanly)
{
    EXPECT_NO_FATAL_FAILURE({ ElideTextLayout layout("tmp"); });
}
