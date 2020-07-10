/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "util.h"
#include "dde/desktopinfo.h"

#include <QWidget>
#include <QWindow>
#include <QDebug>
//for xcb
#include <QtPlatformHeaders/QXcbWindowFunctions>
#define WINDOWS_DESKTOP 0x01000000

namespace DesktopUtil {

void set_desktop_window(QWidget *win)
{
    if (!win)
        return;

    win->winId(); //must be called
    QWindow *window = win->windowHandle();
    if (!window)
        return;

    if (DesktopInfo().waylandDectected()) {
        qDebug() << "wayland set desktop";
        win->setWindowFlags((Qt::WindowFlags)WINDOWS_DESKTOP | Qt::FramelessWindowHint); //to do set Desktop flag
    } else {
        QXcbWindowFunctions::setWmWindowType(window, QXcbWindowFunctions::Desktop);
    }
}
}
