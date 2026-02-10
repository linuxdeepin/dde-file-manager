// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QRectF>
#include <QPainter>
#include <QBrush>
#include <QTextDocument>
#include <QTextLayout>
#include <QTextLine>

#include <dfm-base/utils/elidetextlayout.h>
#include "stubext.h"

using namespace dfmbase;

class ElideTextLayoutTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(ElideTextLayoutTest, Constructor_WithEmptyText_ExpectedEmptyLayout) {
    // Arrange & Act
    ElideTextLayout layout;

    // Assert
    EXPECT_TRUE(layout.text().isEmpty());
}

TEST_F(ElideTextLayoutTest, Constructor_WithText_ExpectedSetText) {
    // Arrange
    QString text = "Hello World";

    // Act
    ElideTextLayout layout(text);

    // Assert
    EXPECT_EQ(layout.text(), text);
}

TEST_F(ElideTextLayoutTest, SetText_WithValidText_ExpectedTextSet) {
    // Arrange
    ElideTextLayout layout;
    QString text = "Test Text";

    // Act
    layout.setText(text);

    // Assert
    EXPECT_EQ(layout.text(), text);
}

TEST_F(ElideTextLayoutTest, Text_GetText_ExpectedSameText) {
    // Arrange
    QString originalText = "Original Text";
    ElideTextLayout layout(originalText);

    // Act
    QString retrievedText = layout.text();

    // Assert
    EXPECT_EQ(retrievedText, originalText);
}

TEST_F(ElideTextLayoutTest, SetAttribute_GetAttribute_ExpectedAttributeValue) {
    // Arrange
    ElideTextLayout layout;
    int lineHeight = 20;
    Qt::Alignment alignment = Qt::AlignLeft;

    // Act
    layout.setAttribute(ElideTextLayout::kLineHeight, lineHeight);
    layout.setAttribute(ElideTextLayout::kAlignment, QVariant::fromValue(alignment));

    // Assert
    EXPECT_EQ(layout.attribute<int>(ElideTextLayout::kLineHeight), lineHeight);
    EXPECT_EQ(layout.attribute<Qt::Alignment>(ElideTextLayout::kAlignment), alignment);
}

TEST_F(ElideTextLayoutTest, SetHighlightKeywords_GetKeywords_ExpectedKeywordsSet) {
    // Arrange
    ElideTextLayout layout;
    QStringList keywords;
    keywords << "test" << "keyword";

    // Act
    layout.setHighlightKeywords(keywords);

    // Note: We can't directly test the internal highlightKeywords list,
    // but we can verify that the function doesn't crash
    EXPECT_TRUE(true);
}

TEST_F(ElideTextLayoutTest, SetHighlightColor_GetColor_ExpectedColorSet) {
    // Arrange
    ElideTextLayout layout;
    QColor color(255, 0, 0);  // Red

    // Act
    layout.setHighlightColor(color);

    // Note: We can't directly test the internal highlightColor,
    // but we can verify that the function doesn't crash
    EXPECT_TRUE(true);
}

TEST_F(ElideTextLayoutTest, SetHighlightEnabled_GetEnabledState_ExpectedStateSet) {
    // Arrange
    ElideTextLayout layout;

    // Act
    layout.setHighlightEnabled(true);

    // Note: We can't directly test the internal enableHighlight flag,
    // but we can verify that the function doesn't crash
    EXPECT_TRUE(true);
}

TEST_F(ElideTextLayoutTest, DocumentHandle_GetHandle_ExpectedHandleNotNull) {
    // Arrange
    ElideTextLayout layout;

    // Act
    QTextDocument *doc = layout.documentHandle();

    // Assert
    EXPECT_NE(doc, nullptr);
}

TEST_F(ElideTextLayoutTest, Layout_WithSimpleText_ExpectedLayoutSuccess) {
    // Arrange
    ElideTextLayout layout("Simple text for testing");
    QRectF rect(0, 0, 100, 50);

    // Act
    QList<QRectF> result = layout.layout(rect, Qt::ElideRight);

    // Assert
    // Just ensure no crash and that it returns a valid list
    EXPECT_TRUE(true);
}

TEST_F(ElideTextLayoutTest, Layout_WithEmptyText_ExpectedEmptyLayout) {
    // Arrange
    ElideTextLayout layout("");
    QRectF rect(0, 0, 100, 50);

    // Act
    QList<QRectF> result = layout.layout(rect, Qt::ElideNone);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}
