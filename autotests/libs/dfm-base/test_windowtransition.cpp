// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_windowtransition.cpp
 * @brief Unit tests for the window transition anchor math
 *
 * The anchor of the space-key preview transition is the selected item's icon
 * rectangle, clipped to the visible viewport and mapped into file manager
 * window coordinates. The pure geometry and the no-Wayland fallback branches
 * of the context/launch entry points are tested here; the Wayland protocol
 * part runs on Treeland and is verified manually.
 */

#include <gtest/gtest.h>

#include <dfm-base/utils/private/windowtransition_p.h>

#include <QIcon>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QWindow>

using namespace dfmbase::windowtransition;

namespace {
QPixmap paddedIconPixmap(const QSize &size, const QRect &content)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.fillRect(content, Qt::red);
    return pixmap;
}
}   // namespace

TEST(WindowTransitionHelperTest, FullyVisibleItemKeepsItsGeometry)
{
    // Item fully inside the viewport: anchor equals the item, shifted by the
    // viewport's origin in the window.
    const QRect anchor = anchorInWindow(QRect(10, 20, 64, 64), QRect(0, 0, 800, 600), QPoint(5, 40));
    EXPECT_EQ(anchor, QRect(15, 60, 64, 64));
}

TEST(WindowTransitionHelperTest, PartiallyVisibleItemAnchorsAtItsVisiblePart)
{
    // Item scrolled half out at the top: only the visible part anchors.
    const QRect anchor = anchorInWindow(QRect(10, -30, 64, 64), QRect(0, 0, 800, 600), QPoint(0, 0));
    EXPECT_EQ(anchor, QRect(10, 0, 64, 34));
}

TEST(WindowTransitionHelperTest, FullyHiddenItemYieldsEmptyAnchor)
{
    // Item scrolled completely out of the viewport: no transition.
    const QRect anchor = anchorInWindow(QRect(10, 700, 64, 64), QRect(0, 0, 800, 600), QPoint(0, 0));
    EXPECT_TRUE(anchor.isEmpty());
}

TEST(WindowTransitionHelperTest, HiddenItemStillEmptyWhenViewportIsOffset)
{
    // The viewport origin in window coordinates must not resurrect a hidden item.
    const QRect anchor = anchorInWindow(QRect(-100, -100, 50, 50), QRect(0, 0, 800, 600), QPoint(200, 300));
    EXPECT_TRUE(anchor.isEmpty());
}

TEST(WindowTransitionHelperTest, ClippingHappensBeforeMapping)
{
    // Clipping is in viewport coordinates, so the window offset applies to the
    // clipped rectangle, not the original one.
    const QRect anchor = anchorInWindow(QRect(-10, 100, 40, 40), QRect(0, 0, 800, 600), QPoint(100, 0));
    EXPECT_EQ(anchor, QRect(100, 100, 30, 40));
}

TEST(WindowTransitionHelperTest, AlphaBoundingRectTrimsTransparentPadding)
{
    QPixmap pixmap = paddedIconPixmap(QSize(40, 40), QRect(10, 5, 20, 30));
    EXPECT_EQ(alphaBoundingRect(pixmap), QRect(10, 5, 20, 30));
}

TEST(WindowTransitionHelperTest, AlphaBoundingRectOnOpaqueImageIsFullRect)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::black);   // no alpha channel
    EXPECT_EQ(alphaBoundingRect(pixmap), QRect(0, 0, 16, 16));
}

TEST(WindowTransitionHelperTest, AlphaBoundingRectOnFullyTransparentImageIsEmpty)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    EXPECT_TRUE(alphaBoundingRect(pixmap).isEmpty());
}

TEST(WindowTransitionHelperTest, PaintedIconRectTrimsIconPadding)
{
    // Icon content occupies the middle 50% of its canvas: the painted rect is
    // the content part, scaled with the rest of the geometry.
    QIcon icon(paddedIconPixmap(QSize(40, 40), QRect(10, 10, 20, 20)));
    QPixmap cropped;
    const QRectF visible = paintedIconRect(icon, QRectF(0, 0, 40, 40), &cropped);
    EXPECT_EQ(visible.toAlignedRect(), QRect(10, 10, 20, 20));
    EXPECT_EQ(cropped.size(), QSize(20, 20));
}

TEST(WindowTransitionHelperTest, PaintedIconRectOnNullIconIsEmpty)
{
    QPixmap cropped;
    EXPECT_TRUE(paintedIconRect(QIcon(), QRectF(0, 0, 100, 100), &cropped).isEmpty());
}

TEST(WindowTransitionHelperTest, CropPixmapToTakesTheSubRectangle)
{
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::red);
    const QPixmap cropped = cropPixmapTo(pixmap, QRectF(0, 0, 100, 100), QRect(10, 20, 50, 30));
    EXPECT_EQ(cropped.size(), QSize(50, 30));
}

TEST(WindowTransitionHelperTest, CropPixmapToHandlesDevicePixelRatio)
{
    // A dpr=2 pixmap depicting a 100x100 logical rect: a 50x100 logical
    // sub-rect (the right half) is 100x200 device pixels.
    QPixmap pixmap(200, 200);
    pixmap.setDevicePixelRatio(2);
    const QPixmap cropped = cropPixmapTo(pixmap, QRectF(0, 0, 100, 100), QRect(50, 0, 50, 100));
    EXPECT_EQ(cropped.size(), QSize(100, 200));
    EXPECT_EQ(cropped.devicePixelRatio(), 2);
}

TEST(WindowTransitionHelperTest, CropPixmapToOnEmptyRectsIsNull)
{
    QPixmap pixmap(10, 10);
    EXPECT_TRUE(cropPixmapTo(pixmap, QRectF(), QRect(0, 0, 5, 5)).isNull());
    EXPECT_TRUE(cropPixmapTo(pixmap, QRectF(0, 0, 10, 10), QRect()).isNull());
}

TEST(WindowTransitionHelperTest, AnchorInWindowWithEmptyViewportIsEmpty)
{
    // An empty viewport clips everything away: no transition.
    EXPECT_TRUE(anchorInWindow(QRect(0, 0, 64, 64), QRect(), QPoint(10, 10)).isEmpty());
}

TEST(WindowTransitionHelperTest, PaintedIconRectOnEmptyCellIsEmpty)
{
    QPixmap cropped;
    QIcon icon(paddedIconPixmap(QSize(40, 40), QRect(0, 0, 40, 40)));
    EXPECT_TRUE(paintedIconRect(icon, QRectF(), &cropped).isEmpty());
}

TEST(WindowTransitionHelperTest, ContextCreateWithoutWindowOrAnchorReturnsNull)
{
    // Both are rejected before anything touches the Wayland managers.
    EXPECT_EQ(WindowTransitionContext::create(nullptr, QRect(0, 0, 10, 10), QPixmap()), nullptr);

    QWindow window;
    EXPECT_EQ(WindowTransitionContext::create(&window, QRect(), QPixmap()), nullptr);
}

TEST(WindowTransitionHelperTest, ContextCreateWithoutWaylandReturnsNull)
{
    // The offscreen platform has no Wayland native interface, so the managers
    // never bind and create() must fail cleanly instead of opening a second
    // connection.
    QWindow window;
    EXPECT_EQ(WindowTransitionContext::create(&window, QRect(0, 0, 64, 64), QPixmap()), nullptr);
}

TEST(WindowTransitionHelperTest, LaunchCreateWithoutSourceOrProviderReturnsNull)
{
    QUrl url = QUrl::fromLocalFile("/tmp/ut-windowtransition.txt");
    WindowTransitionLaunch::AnchorProvider provider = [](const QUrl &, QRect *, QPixmap *) {
        return false;
    };
    WindowTransitionLaunch::TokenCallback callback = [](const QString &, const QString &) {
    };

    EXPECT_EQ(WindowTransitionLaunch::create(nullptr, url, provider, callback), nullptr);
    EXPECT_EQ(WindowTransitionLaunch::create(nullptr, url, WindowTransitionLaunch::AnchorProvider(), callback), nullptr);
    EXPECT_EQ(WindowTransitionLaunch::create(nullptr, url, provider, WindowTransitionLaunch::TokenCallback()), nullptr);

    QWindow window;
    EXPECT_EQ(WindowTransitionLaunch::create(&window, url, provider, callback), nullptr);
}

TEST(WindowTransitionHelperTest, ActivateWithTokenWithoutWindowOrTokenReturnsFalse)
{
    EXPECT_FALSE(activateWithToken(nullptr, QString()));
    EXPECT_FALSE(activateWithToken(nullptr, QStringLiteral("token-ut")));
}

TEST(WindowTransitionHelperTest, ActivateWithTokenWithoutWaylandReturnsFalse)
{
    // No xdg-activation manager on the offscreen platform: the caller falls
    // back to a normal activation.
    QWindow window;
    EXPECT_FALSE(activateWithToken(&window, QStringLiteral("token-ut")));
}

TEST(WindowTransitionHelperTest, BindManagersIsCallableWithoutWayland)
{
    // Smoke test: must not crash and must not create anything without the
    // Wayland platform plugin.
    EXPECT_NO_THROW(bindManagers());
}
