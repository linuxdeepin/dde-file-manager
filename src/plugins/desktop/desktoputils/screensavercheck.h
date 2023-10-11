// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSAVERCHECK_H
#define SCREENSAVERCHECK_H

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

}
#endif // SCREENSAVERCHECK_H
