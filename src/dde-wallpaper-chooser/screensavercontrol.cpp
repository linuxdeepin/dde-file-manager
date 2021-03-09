/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             xinglinkun<xinglinkun@uniontech.com>
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
#include "screensavercontrol.h"

#include <QDebug>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusMessage>
#include <dgiosettings.h>

#define DESKTOP_CAN_SCREENSAVER "DESKTOP_CAN_SCREENSAVER"

namespace ScreenSaverCtrlFunction {
bool needShowScreensaver()
{
#ifndef DISABLE_SCREENSAVER
    //1 判断环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if ((env.contains(DESKTOP_CAN_SCREENSAVER) && env.value(DESKTOP_CAN_SCREENSAVER).startsWith("N")))
    {
        qWarning() << "System environment variables do not support screen savers";
        return false;
    }

    //2 判断是否安装屏保程序
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListActivatableNames");
    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if(response.type() == QDBusMessage::ReplyMessage
            && response.arguments().takeFirst().toList().contains("com.deepin.ScreenSaver")){
        qDebug() << "com.deepin.ScreenSaver is ok";
    }else {
        qWarning() << "The screen saver is uninstalled";
        return false;
    }

    //3 Gsetting 判断屏保是否可用...
    DGioSettings desktopSettings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
    if (desktopSettings.keys().contains("show-screen-saver") && false == desktopSettings.value("show-screen-saver").toBool()) {
        qWarning() << "Gsetting show-screen-saver is false";
        return false;
    }

    return true;
#else
    return false;
#endif
}
}
