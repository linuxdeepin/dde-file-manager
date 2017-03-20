/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "display.h"

#include <QScreen>
#include <QApplication>

#include <dbus/dbusdisplay.h>

Display::Display(QObject *parent) : QObject(parent)
{
#ifdef DDE_DBUS_DISPLAY
    m_display = new DBusDisplay(this);
    connect(m_display, &DBusDisplay::PrimaryRectChanged, this, [ = ]() {
        auto primaryName = m_display->primary();
        for (auto screen : qApp->screens()) {
            if (screen && screen->name() == primaryName) {
                emit primaryScreenChanged(screen);
                return;
            }
        }
        qCritical() << "Can not find" << primaryName << qApp->screens();
    });
#else
    connect(qApp, &QApplication::primaryScreenChanged,
            this, &Display::primaryScreenChanged);
#endif
}

QScreen *Display::primaryScreen()
{
#ifdef DDE_DBUS_DISPLAY
    auto primaryName = m_display->primary();
    for (auto screen : qApp->screens()) {
        if (screen && screen->name() == primaryName) {
            return screen;
        }
    }
    qCritical() << "Can not find" << primaryName;
    return qApp->primaryScreen();
#else
    return qApp->primaryScreen();
#endif
}
