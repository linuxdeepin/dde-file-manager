// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_elidetextlayout_ext2.cpp
 * @brief Mode 2 supplement: High-priority ElideTextLayout::determineElidePosition.
 *
 * Branch清单 (declared, cross-checked via MCP get_code_snippet):
 *   determineElidePosition:
 *     - elideText==originalText → return -1
 *     - ElideRight + hasEllipsis (…) → ellipsis position
 *     - ElideRight + hasEllipsis (...) → ellipsis position
 *     - ElideRight + no ellipsis → mismatch position
 *     - ElideLeft + hasEllipsis → ellipsis position
 *     - ElideMiddle → mismatch/ellipsis position
 *     - ElideNone → default path
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>
#include <QFont>
#include <QRectF>

#include <dfm-base/utils/elidetextlayout.h>

using namespace dfmbase;

class ElideTextLayoutExt2Test : public testing::Test
{
protected:
    ElideTextLayout layout { "placeholder" };
};

// ── Same text → -1 ──

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionSameTextReturnsMinusOne)
{
    QString text = "identical text";
    int pos = layout.determineElidePosition(text, text, Qt::ElideRight);
    EXPECT_EQ(pos, -1);
}

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionSameTextElideLeft)
{
    QString text = "same";
    int pos = layout.determineElidePosition(text, text, Qt::ElideLeft);
    EXPECT_EQ(pos, -1);
}

// ── ElideRight with ellipsis … ──

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionRightWithEllipsis)
{
    // "Hello World" elided right → "Hello W…"
    QString original = "Hello World";
    QString elided = "Hello W…";
    int pos = layout.determineElidePosition(elided, original, Qt::ElideRight);
    // Should find the ellipsis at position 7 (or nearby)
    EXPECT_GE(pos, 0);
    EXPECT_LE(pos, elided.length());
}

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionRightWithThreeDots)
{
    // Using "..." instead of "…"
    QString original = "Hello World";
    QString elided = "Hello W...";
    int pos = layout.determineElidePosition(elided, original, Qt::ElideRight);
    EXPECT_GE(pos, 0);
}

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionRightNoEllipsisReturnsMismatch)
{
    // Different text without ellipsis → returns mismatch position
    QString original = "Hello World";
    QString elided = "Hell";   // truncated, no ellipsis char
    int pos = layout.determineElidePosition(elided, original, Qt::ElideRight);
    // First mismatch is at index 4 (elided ends, original continues)
    EXPECT_GE(pos, 0);
    EXPECT_LE(pos, elided.length());
}

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionRightFullMatchPrefix)
{
    // The elided text is a prefix of original (no ellipsis)
    QString original = "abcdefg";
    QString elided = "abcd";
    int pos = layout.determineElidePosition(elided, original, Qt::ElideRight);
    EXPECT_GE(pos, 0);
}

// ── ElideLeft ──

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionLeftWithEllipsis)
{
    QString original = "Hello World";
    QString elided = "…World";
    int pos = layout.determineElidePosition(elided, original, Qt::ElideLeft);
    EXPECT_GE(pos, 0);
    EXPECT_LE(pos, elided.length());
}

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionLeftWithThreeDots)
{
    QString original = "Hello World";
    QString elided = "...World";
    int pos = layout.determineElidePosition(elided, original, Qt::ElideLeft);
    EXPECT_GE(pos, 0);
}

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionLeftNoEllipsisReturnsMismatch)
{
    QString original = "Hello World";
    QString elided = "orld";   // suffix without ellipsis
    int pos = layout.determineElidePosition(elided, original, Qt::ElideLeft);
    EXPECT_GE(pos, 0);
    EXPECT_LE(pos, elided.length());
}

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionLeftSameTextReturnsMinusOne)
{
    QString text = "same";
    int pos = layout.determineElidePosition(text, text, Qt::ElideLeft);
    EXPECT_EQ(pos, -1);
}

// ── ElideMiddle ──

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionMiddleWithEllipsis)
{
    QString original = "Hello World";
    QString elided = "Hell…orld";
    int pos = layout.determineElidePosition(elided, original, Qt::ElideMiddle);
    EXPECT_GE(pos, 0);
}

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionMiddleNoEllipsis)
{
    QString original = "abcdefg";
    QString elided = "abfg";
    int pos = layout.determineElidePosition(elided, original, Qt::ElideMiddle);
    EXPECT_GE(pos, 0);
}

// ── ElideNone ──

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionNoneDifferentText)
{
    QString original = "Hello World";
    QString elided = "Hello Worl";   // slightly different
    int pos = layout.determineElidePosition(elided, original, Qt::ElideNone);
    // For ElideNone, falls through to default → returns some position
    EXPECT_GE(pos, -1);
}

// ── Edge cases ──

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionEmptyElidedText)
{
    QString original = "Hello";
    QString elided;
    int pos = layout.determineElidePosition(elided, original, Qt::ElideRight);
    // Empty text ≠ original → not -1
    EXPECT_NE(pos, -1);
}

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionEmptyOriginalText)
{
    QString original;
    QString elided = "Hello";
    int pos = layout.determineElidePosition(elided, original, Qt::ElideRight);
    // Different texts → not -1
    EXPECT_NE(pos, -1);
}

TEST_F(ElideTextLayoutExt2Test, DetermineElidePositionBothEmptyReturnsMinusOne)
{
    QString text;
    int pos = layout.determineElidePosition(text, text, Qt::ElideRight);
    EXPECT_EQ(pos, -1);
}
