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
