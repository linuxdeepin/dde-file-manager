// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
// #    include <dde-shell/dlayershellwindow.h>
#endif

namespace ddplugin_desktop_util {

static inline void setDesktopWindow(QWidget *w)
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
