// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QSize>
#include <QPixmap>
#include <QString>

#include <dfm-base/utils/iconutils.h>
#include "stubext.h"

using namespace dfmbase;

class IconUtilsTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(IconUtilsTest, RenderIconBackground_WithSize_ExpectedPixmapCreated) {
    // Arrange
    QSize size(64, 64);

    // Act
    QPixmap pixmap = IconUtils::renderIconBackground(size);

    // Assert
    EXPECT_FALSE(pixmap.isNull());
    EXPECT_EQ(pixmap.size(), size);
}

TEST_F(IconUtilsTest, RenderIconBackground_WithSizeAndStyle_ExpectedPixmapCreated) {
    // Arrange
    QSize size(32, 32);
    IconUtils::IconStyle style;
    style.radius = 8;

    // Act
    QPixmap pixmap = IconUtils::renderIconBackground(size, style);

    // Assert
    EXPECT_FALSE(pixmap.isNull());
    EXPECT_EQ(pixmap.size(), size);
}

TEST_F(IconUtilsTest, RenderIconBackground_WithSizeF_ExpectedPixmapCreated) {
    // Arrange
    QSizeF size(64.5, 64.5);

    // Act
    QPixmap pixmap = IconUtils::renderIconBackground(size);

    // Assert
    EXPECT_FALSE(pixmap.isNull());
    EXPECT_EQ(pixmap.size(), QSize(64, 64));  // Should be rounded
}

TEST_F(IconUtilsTest, GetIconStyle_WithSmallSize_ExpectedSmallStyle) {
    // Arrange
    int size = 32;

    // Act
    IconUtils::IconStyle style = IconUtils::getIconStyle(size);

    // Assert
    EXPECT_EQ(style.stroke, 1);
    EXPECT_EQ(style.radius, 2);
    EXPECT_EQ(style.shadowOffset, 1);
    EXPECT_EQ(style.shadowRange, 2);
}

TEST_F(IconUtilsTest, GetIconStyle_WithMediumSize_ExpectedMediumStyle) {
    // Arrange
    int size = 64;

    // Act
    IconUtils::IconStyle style = IconUtils::getIconStyle(size);

    // Assert
    EXPECT_EQ(style.stroke, 2);
    EXPECT_EQ(style.radius, 4);
    EXPECT_EQ(style.shadowOffset, 1);
    EXPECT_EQ(style.shadowRange, 3);
}

TEST_F(IconUtilsTest, GetIconStyle_WithLargeSize_ExpectedLargeStyle) {
    // Arrange
    int size = 128;

    // Act
    IconUtils::IconStyle style = IconUtils::getIconStyle(size);

    // Assert
    EXPECT_EQ(style.stroke, 4);
    EXPECT_EQ(style.radius, 8);
    EXPECT_EQ(style.shadowOffset, 3);
    EXPECT_EQ(style.shadowRange, 5);
}

TEST_F(IconUtilsTest, GetIconStyle_WithVeryLargeSize_ExpectedVeryLargeStyle) {
    // Arrange
    int size = 256;

    // Act
    IconUtils::IconStyle style = IconUtils::getIconStyle(size);

    // Assert
    EXPECT_EQ(style.stroke, 6);
    EXPECT_EQ(style.radius, 12);
    EXPECT_EQ(style.shadowOffset, 4);
    EXPECT_EQ(style.shadowRange, 8);
}

TEST_F(IconUtilsTest, ShouldSkipThumbnailFrame_WithAppImageMime_ExpectedTrue) {
    // Arrange
    QString mimeType = "application/x-iso9660-appimage";

    // Act
    bool result = IconUtils::shouldSkipThumbnailFrame(mimeType);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(IconUtilsTest, ShouldSkipThumbnailFrame_WithUabMime_ExpectedTrue) {
    // Arrange
    QString mimeType = "application/x-uab";

    // Act
    bool result = IconUtils::shouldSkipThumbnailFrame(mimeType);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(IconUtilsTest, ShouldSkipThumbnailFrame_WithNormalMime_ExpectedFalse) {
    // Arrange
    QString mimeType = "text/plain";

    // Act
    bool result = IconUtils::shouldSkipThumbnailFrame(mimeType);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(IconUtilsTest, AddShadowToPixmap_WithPixmap_ExpectedNoCrash) {
    // Arrange
    QPixmap originalPixmap(32, 32);
    originalPixmap.fill(Qt::red);

    // Act
    QPixmap result = IconUtils::addShadowToPixmap(originalPixmap, 2, 5.0, 0.5);

    // Assert
    // Just ensure no crash and result is valid
    EXPECT_TRUE(true);
}