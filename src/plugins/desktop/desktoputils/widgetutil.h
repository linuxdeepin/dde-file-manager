// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WIDGETUTIL_H
#define WIDGETUTIL_H

#include "screensavercheck.h"

#include <dfm-base/utils/windowutils.h>

#include <QWidget>
#include <QApplication>
#include <QWindow>
#include <QDebug>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <QtPlatformHeaders/QXcbWindowFunctions>
#else
#    include <qpa/qplatformwindow.h>
#    include <qpa/qplatformwindow_p.h>

#    include <xcb/xcb.h>
#    include <xcb/xcb_ewmh.h>
#    include <dde-shell/dlayershellwindow.h>
#endif

namespace ddplugin_desktop_util {

static inline void setDesktopWindowOld(QWidget *w)
{
    if (!w)
        return;

    w->winId();   // must be called
    QWindow *window = w->windowHandle();
    if (!window) {
        qWarning() << w << "windowHandle is null";
        return;
    }

    if (DFMBASE_NAMESPACE::WindowUtils::isWayLand()) {
        qDebug() << "wayland set desktop";
        w->setWindowFlags(Qt::FramelessWindowHint);
        w->setAttribute(Qt::WA_NativeWindow);
        window->setProperty("_d_dwayland_window-type", "desktop");
    } else {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QXcbWindowFunctions::setWmWindowType(window, QXcbWindowFunctions::Desktop);
#else
        // NOTE: invalid -> (https://invent.kde.org/plasma/libplasma/-/commit/764417856eef31ecdfd8d77d6802647f4d241a83)
        // window->setProperty("_q_xcb_wm_window_type", QNativeInterface::Private::QXcbWindow::Desktop);

        // dde-shell: LayerShellEmulation::onLayerChanged
        auto xcbWindow = dynamic_cast<QNativeInterface::Private::QXcbWindow *>(window->handle());
        if (xcbWindow)
            xcbWindow->setWindowType(QNativeInterface::Private::QXcbWindow::Desktop);
#endif
    }
}

static inline void setDesktopWindow(QWidget *w)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    setDesktopWindowOld(w);
#else
    if (!w)
        return;

    w->winId();   // must be called
    QWindow *window = w->windowHandle();
    if (!window) {
        qWarning() << w << "windowHandle is null";
        return;
    }

    // 使用 dde-shell 完成桌面窗口设置
    DS_USE_NAMESPACE
    auto layerShellWindow = DLayerShellWindow::get(window);
    if (!layerShellWindow) {
        qWarning() << w << "DLayerShellWindow is null! Use setDesktopWindowOld";
        setDesktopWindowOld(w);
    } else {
        qInfo() << "set desktop by dde-shell";
        // 窗管这边要特判来做动画  所以要调用setScope
        layerShellWindow->setScope("dde-shell/desktop");
        DLayerShellWindow::Anchors anchors = static_cast<DLayerShellWindow::Anchors>(
                DLayerShellWindow::AnchorTop | DLayerShellWindow::AnchorBottom | DLayerShellWindow::AnchorLeft | DLayerShellWindow::AnchorRight);
        layerShellWindow->setAnchors(anchors);
        layerShellWindow->setLeftMargin(0);
        layerShellWindow->setRightMargin(0);
        layerShellWindow->setTopMargin(0);
        layerShellWindow->setBottomMargin(0);
        layerShellWindow->setExclusiveZone(0);
        layerShellWindow->setLayer(DLayerShellWindow::LayerBackground);
        layerShellWindow->setKeyboardInteractivity(DLayerShellWindow::KeyboardInteractivityOnDemand);
        layerShellWindow->setScreenConfiguration(DLayerShellWindow::ScreenFromQWindow);

        qInfo() << "Desktop base info:"
                << "Scope: " << layerShellWindow->scope()
                << "Anchors: " << layerShellWindow->anchors()
                << "Margins: " << layerShellWindow->topMargin() << layerShellWindow->leftMargin()
                << layerShellWindow->bottomMargin() << layerShellWindow->rightMargin()
                << "ExclusiveZone: " << layerShellWindow->exclusionZone()
                << "Layer: " << layerShellWindow->layer()
                << "KeyboardInteractivity: " << layerShellWindow->keyboardInteractivity()
                << "ScreenConfiguration: " << layerShellWindow->screenConfiguration();
    }
#endif
}

static inline void setPrviewWindow(QWidget *w)
{
    if (!w)
        return;

    w->setWindowFlags(w->windowFlags() | Qt::BypassWindowManagerHint | Qt::WindowDoesNotAcceptFocus);
    if (DFMBASE_NAMESPACE::WindowUtils::isWayLand()) {
        w->winId();   // must be called
        QWindow *window = w->windowHandle();
        if (!window) {
            qWarning() << w << "windowHandle is null";
            return;
        }
        qDebug() << "wayland set role dock";
        window->setProperty("_d_dwayland_window-type", "wallpaper");
    }
}
}

#endif   // WIDGETUTIL_H
