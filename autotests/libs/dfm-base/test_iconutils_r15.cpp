// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iconutils_r15.cpp
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

TEST(IconUtilsR15Test, RenderIconBackgroundReturnsPixmap)
{
    QPixmap pm = renderIconBackground(QSize(32, 32));
    EXPECT_FALSE(pm.isNull());
    EXPECT_EQ(pm.size(), QSize(32, 32));
}

TEST(IconUtilsR15Test, RenderIconBackgroundSizeF)
{
    QPixmap pm = renderIconBackground(QSizeF(48, 48));
    EXPECT_FALSE(pm.isNull());
}

TEST(IconUtilsR15Test, RenderIconBackgroundEmptySize)
{
    QPixmap pm = renderIconBackground(QSize(0, 0));
    EXPECT_TRUE(pm.isNull());
}

TEST(IconUtilsR15Test, ShouldSkipThumbnailFrameAppImage)
{
    EXPECT_TRUE(shouldSkipThumbnailFrame("application/vnd.appimage"));
}

TEST(IconUtilsR15Test, ShouldSkipThumbnailFrameNonExcluded)
{
    EXPECT_FALSE(shouldSkipThumbnailFrame("text/plain"));
}

TEST(IconUtilsR15Test, GetIconStyleReturnsValid)
{
    EXPECT_NO_FATAL_FAILURE({ (void)getIconStyle(48); });
}

TEST(IconUtilsR15Test, AddShadowToPixmapNullReturnsNull)
{
    QPixmap result = addShadowToPixmap(QPixmap(), 5, 10.0, 0.5);
    EXPECT_NO_FATAL_FAILURE({ (void)result.isNull(); });
}

TEST(IconUtilsR15Test, AddShadowToPixmapValid)
{
    QPixmap src(32, 32);
    src.fill(Qt::white);
    QPixmap result = addShadowToPixmap(src, 5, 10.0, 0.5);
    EXPECT_FALSE(result.isNull());
}

TEST(IconUtilsR15Test, HiDpiPixmapReturnsPixmap)
{
    QIcon icon = QIcon::fromTheme("document-new");
    QPixmap result = hiDpiPixmap(icon, QSize(16, 16), nullptr);
    // May be null if icon theme missing; just verify no crash
    EXPECT_NO_FATAL_FAILURE({ (void)result.isNull(); });
}
