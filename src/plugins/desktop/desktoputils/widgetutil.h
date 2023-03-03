// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WIDGETUTIL_H
#define WIDGETUTIL_H

#include <dfm-base/utils/windowutils.h>

#include <QWidget>
#include <QApplication>
#include <QWindow>
#include <QtPlatformHeaders/QXcbWindowFunctions>
#include <QDBusMessage>
#include <QProcessEnvironment>
#include <QGSettings>
#include <QDBusConnection>
#include <QDebug>

namespace ddplugin_desktop_util {

static inline bool enableScreensaver()
{
    static const char *envKey = "DESKTOP_CAN_SCREENSAVER";
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if ((env.contains(envKey) && env.value(envKey).startsWith("N"))) {
        qWarning() << "System environment variables do not support screen savers";
        return false;
    }

    qInfo() << "check com.deepin.ScreenSaver";
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListActivatableNames");
    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage && !response.arguments().isEmpty()
        && response.arguments().first().toList().contains("com.deepin.ScreenSaver")) {
        qDebug() << "com.deepin.ScreenSaver is ok";
    } else {
        qWarning() << "The screen saver is uninstalled";
        return false;
    }

    QGSettings desktopSettings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
    if (desktopSettings.keys().contains("showScreenSaver") && !desktopSettings.get("showScreenSaver").toBool()) {
        qWarning() << "Gsetting show-screen-saver is false";
        return false;
    }

    return true;
}

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
