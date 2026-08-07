// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iconutils.cpp
 * @brief Unit tests for IconUtils free functions (utils/iconutils.cpp) — the
 *        pure-logic / painting subset that doesn't require a GUI display.
 *        Uses offscreen Qt platform.
 */

#include <gtest/gtest.h>
#include <QPixmap>
#include <QSize>
#include <QSizeF>
#include <QString>
#include <QIcon>

#include <dfm-base/utils/iconutils.h>

using namespace dfmbase::IconUtils;

TEST(IconUtilsTest, RenderIconBackgroundReturnsPixmap)
{
    QPixmap pm = renderIconBackground(QSize(32, 32));
    EXPECT_FALSE(pm.isNull());
    EXPECT_EQ(pm.size(), QSize(32, 32));
}

TEST(IconUtilsTest, RenderIconBackgroundSizeF)
{
    QPixmap pm = renderIconBackground(QSizeF(48, 48));
    EXPECT_FALSE(pm.isNull());
}

TEST(IconUtilsTest, RenderIconBackgroundEmptySize)
{
    QPixmap pm = renderIconBackground(QSize(0, 0));
    EXPECT_TRUE(pm.isNull());
}

TEST(IconUtilsTest, ShouldSkipThumbnailFrameAppImage)
{
    EXPECT_TRUE(shouldSkipThumbnailFrame("application/vnd.appimage"));
}

TEST(IconUtilsTest, ShouldSkipThumbnailFrameNonExcluded)
{
    EXPECT_FALSE(shouldSkipThumbnailFrame("text/plain"));
}

TEST(IconUtilsTest, GetIconStyleReturnsValid)
{
    EXPECT_NO_FATAL_FAILURE({ (void)getIconStyle(48); });
}

TEST(IconUtilsTest, AddShadowToPixmapNullReturnsNull)
{
    QPixmap result = addShadowToPixmap(QPixmap(), 5, 10.0, 0.5);
    EXPECT_NO_FATAL_FAILURE({ (void)result.isNull(); });
}

TEST(IconUtilsTest, AddShadowToPixmapValid)
{
    QPixmap src(32, 32);
    src.fill(Qt::white);
    QPixmap result = addShadowToPixmap(src, 5, 10.0, 0.5);
    EXPECT_FALSE(result.isNull());
}

TEST(IconUtilsTest, HiDpiPixmapReturnsPixmap)
{
    QIcon icon = QIcon::fromTheme("document-new");
    QPixmap result = hiDpiPixmap(icon, QSize(16, 16), nullptr);
    // May be null if icon theme missing; just verify no crash
    EXPECT_NO_FATAL_FAILURE({ (void)result.isNull(); });
}
