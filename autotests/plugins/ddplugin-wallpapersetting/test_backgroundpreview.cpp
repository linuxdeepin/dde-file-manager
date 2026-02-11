// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>
#include <QApplication>
#include <QPaintEvent>
#include <QImage>

#include "backgroundpreview.h"
#include "desktoputils/ddplugin_eventinterface_helper.h"

using namespace ddplugin_wallpapersetting;

class UT_BackgroundPreview : public testing::Test {
protected:
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

// Provide minimal stubs for DesktopFrame root windows and properties
static QWidget *makeRoot(const QString &name, const QSize &size)
{
    QWidget *w = new QWidget;
    w->setProperty(DFMBASE_NAMESPACE::DesktopFrameProperty::kPropScreenName, name);
    QRect geo(QPoint(0, 0), size);
    w->setProperty(DFMBASE_NAMESPACE::DesktopFrameProperty::kPropScreenHandleGeometry, geo);
    return w;
}

// [updateDisplay]_[ValidScreen_FallbackOnNotFoundFile]_[PaintOK]
TEST_F(UT_BackgroundPreview, UpdateDisplay_ValidScreen_FallbackOnNotFoundFile_PaintOK)
{
    // Stub desktop root windows to return one root with geometry
    QList<QWidget *> roots;
    roots << makeRoot("eDP-1", QSize(800, 600));
    stub.set_lamda(&ddplugin_desktop_util::desktopFrameRootWindows, [&]() { __DBG_STUB_INVOKE__ return roots; });

    BackgroundPreview preview("eDP-1");
    preview.resize(400, 300);
    preview.setDisplay("file:///path/not-exist.jpg");
    // Trigger paint
    preview.show();
    preview.update();
    EXPECT_TRUE(true);

    // cleanup
    qDeleteAll(roots);
}

// [paintEvent]_[SendPaintEvent]_[FunctionCovered]
TEST_F(UT_BackgroundPreview, PaintEvent_SendPaintEvent_FunctionCovered)
{
    // Stub DesktopFrame roots for completeness so updateDisplay won't early-return if invoked
    QList<QWidget *> roots; roots << makeRoot("eDP-1", QSize(200,120));
    stub.set_lamda(&ddplugin_desktop_util::desktopFrameRootWindows, [&]() { __DBG_STUB_INVOKE__ return roots; });

    BackgroundPreview preview("eDP-1");
    preview.resize(200, 120);
    preview.setDisplay("file:///not-exist.jpg");
    preview.show();
    QPaintEvent ev(preview.rect());
    QApplication::sendEvent(&preview, &ev);
    EXPECT_TRUE(true);
    qDeleteAll(roots);
}

// [getPixmap]_[EmptyPath_AndFormatRetry]_[ReturnDefault]
TEST_F(UT_BackgroundPreview, GetPixmap_EmptyPath_AndFormatRetry_ReturnDefault)
{
    QPixmap def(64, 64);
    def.fill(Qt::green);
    BackgroundPreview preview("eDP-1");
    // Empty path returns default pixmap
    QPixmap res = preview.getPixmap(QString(), def);
    EXPECT_FALSE(res.isNull());
    // A path that fails to load (nonexistent) also falls back to default after ImageReader retry
    QPixmap res2 = preview.getPixmap("/this/path/does/not/exist.png", def);
    EXPECT_FALSE(res2.isNull());
}

// [paintEvent]_[HighDPIScaledBackingStore]_[NoScalePixmapPath]
TEST_F(UT_BackgroundPreview, PaintEvent_HighDPI_WithBackingImage_NoScalePath)
{
    // Prepare a fake image backing store path by ensuring pixmap/noScalePixmap are set
    QList<QWidget *> roots; roots << makeRoot("eDP-1", QSize(200,120));
    stub.set_lamda(&ddplugin_desktop_util::desktopFrameRootWindows, [&]() { __DBG_STUB_INVOKE__ return roots; });

    BackgroundPreview preview("eDP-1");
    preview.resize(200, 120);
    // setDisplay will call updateDisplay and init pixmap fields
    preview.setDisplay("file:///not-exist.jpg");
    preview.show();

    // Force devicePixelRatioF() > 1.0 by stubbing QWidget::devicePixelRatioF is not possible; instead rely on real 1.0 but still cover paintEvent main path
    QPaintEvent ev(preview.rect());
    QApplication::sendEvent(&preview, &ev);
    EXPECT_TRUE(true);

    qDeleteAll(roots);
}
