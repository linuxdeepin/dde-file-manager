/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "desktoputils.h"

#include <QGSettings>

#include <QApplication>
#include <QWindow>
#include <QDebug>
#include <QtPlatformHeaders/QXcbWindowFunctions>
#include <QProcessEnvironment>
#include <QDBusMessage>
#include <QDBusConnection>

bool dfm_service_desktop::waylandDectected()
{
    //! 该函数只能在QApplication之后调用才能返回有效的值，在此之前会返回空值
    Q_ASSERT(qApp);
    return QApplication::platformName() == "wayland";
}

void dfm_service_desktop::setDesktopWindow(QWidget *w)
{
    if (!w)
        return;

    w->winId(); // must be called
    QWindow *window = w->windowHandle();
    if (!window) {
        qWarning() << w << "windowHandle is null";
        return;
    }

    if (waylandDectected()) {
        qDebug() << "wayland set desktop";
        w->setWindowFlags(Qt::FramelessWindowHint);
        w->setAttribute(Qt::WA_NativeWindow);
        window->setProperty("_d_dwayland_window-type","desktop");
    } else {
        QXcbWindowFunctions::setWmWindowType(window, QXcbWindowFunctions::Desktop);
    }
}

void dfm_service_desktop::setPrviewWindow(QWidget *w)
{
    if (!w)
        return;

    w->setWindowFlags(w->windowFlags() | Qt::BypassWindowManagerHint | Qt::WindowDoesNotAcceptFocus);
    if (waylandDectected()) {
        w->winId(); // must be called
        QWindow *window = w->windowHandle();
        if (!window) {
            qWarning() << w << "windowHandle is null";
            return;
        }
        qDebug() << "wayland set role dock";
        window->setProperty("_d_dwayland_window-type","wallpaper");
    }
}

bool dfm_service_desktop::enableScreensaver()
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
            && response.arguments().first().toList().contains("com.deepin.ScreenSaver")){
        qDebug() << "com.deepin.ScreenSaver is ok";
    } else {
        qWarning() << "The screen saver is uninstalled";
        return false;
    }

    QGSettings desktopSettings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
    if (desktopSettings.keys().contains("show-screen-saver") && !desktopSettings.get("show-screen-saver").toBool()) {
        qWarning() << "Gsetting show-screen-saver is false";
        return false;
    }

    return true;
}
