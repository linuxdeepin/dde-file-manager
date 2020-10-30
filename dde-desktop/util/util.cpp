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
#include <QPixmap>
#include <QPainter>
#include <QImageReader>
#include <QCursor>
#include <QWidget>
#include <QWindow>
#include <QDebug>

//#include <X11/Xcursor/Xcursor.h>
//for xcb
#include <QtPlatformHeaders/QXcbWindowFunctions>

namespace DesktopUtil
{
void set_desktop_window(QWidget *w)
{
    if (!w)
        return;

    w->winId(); //must be called
    QWindow *window = w->windowHandle();
    if (!window){
        qWarning() << w << "windowHandle is null";
        return;
    }

    if (DesktopInfo().waylandDectected()) {
        qDebug() << "wayland set desktop";
        w->setWindowFlags(Qt::FramelessWindowHint);
        w->setAttribute(Qt::WA_NativeWindow);
        window->setProperty("_d_dwayland_window-type","desktop");
    } else {
        QXcbWindowFunctions::setWmWindowType(window, QXcbWindowFunctions::Desktop);
    }
}

#if 0
QCursor *loadQCursorFromX11Cursor(const char* theme, const char* cursorName, int cursorSize)
{
    if (theme == nullptr || cursorName == nullptr || cursorSize <= 0)
        return nullptr;

    XcursorImages *images = XcursorLibraryLoadImages(cursorName, theme, cursorSize);
    if (images == nullptr || images->images[0] == nullptr) {
        qWarning() << "loadCursorFalied, theme =" << theme << ", cursorName=" << cursorName;
        return nullptr;
    }

    const int imgW = images->images[0]->width;
    const int imgH = images->images[0]->height;
    QImage img((const uchar*)images->images[0]->pixels, imgW, imgH, QImage::Format_ARGB32);
    QPixmap pixmap = QPixmap::fromImage(img);
    QCursor *cursor = new QCursor(pixmap, images->images[0]->xhot, images->images[0]->yhot);
    delete images;
    return cursor;
}
#endif

void set_prview_window(QWidget *w)
{
    if (!w)
        return;

    w->setWindowFlags(w->windowFlags() | Qt::BypassWindowManagerHint | Qt::WindowDoesNotAcceptFocus);
    if (DesktopInfo().waylandDectected()) {
        w->winId(); //must be called
        QWindow *window = w->windowHandle();
        if (!window){
            qWarning() << w << "windowHandle is null";
            return;
        }
        qDebug() << "wayland set role dock";
        window->setProperty("_d_dwayland_window-type","wallpaper");
    }
}
}

