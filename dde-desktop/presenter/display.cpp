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

QScreen * GetPrimaryScreen()
{
    return Display::instance()->primaryScreen();
}

Display::Display(QObject *parent) : QObject(parent)
{
#ifdef DDE_DBUS_DISPLAY
    m_display = new DBusDisplay(this);
    connect(m_display, &DBusDisplay::PrimaryRectChanged, this, [ = ]() {
        auto primaryName = m_display->primary();

        emit primaryScreenChanged(primaryScreen());
        qCritical() << "find primaryScreen:" << primaryName << primaryScreen();
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
    static QPair<QString, QScreen *> s_primaryScreen;
    if (s_primaryScreen.first.isEmpty()) {
        s_primaryScreen.first = primaryName;
        s_primaryScreen.second = qApp->primaryScreen();
    }

    if (m_display->primary() == s_primaryScreen.first) {
        qCritical() << "primaryScreen:" << s_primaryScreen.second;
        return s_primaryScreen.second;
    }

    for (auto screen : qApp->screens()) {
        if (screen != s_primaryScreen.second) {
            qCritical() << "primaryScreen:" << screen;
            return screen;
        }
    }

    qCritical() << "primaryScreen:" << qApp->primaryScreen();
    return qApp->primaryScreen();
#else
    return qApp->primaryScreen();
#endif
}
