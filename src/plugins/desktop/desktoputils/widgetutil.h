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
#include <QtPlatformHeaders/QXcbWindowFunctions>
#include <QDebug>

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
        QXcbWindowFunctions::setWmWindowType(window, QXcbWindowFunctions::Desktop);
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

#endif // WIDGETUTIL_H
