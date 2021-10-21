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
#ifndef AUTOACTIVATEWINDOW_P_H
#define AUTOACTIVATEWINDOW_P_H

#include "autoactivatewindow.h"

#include <QTimer>

#include <xcb/xcb.h>

class AutoActivateWindowPrivate : public QObject
{
    Q_OBJECT
public:
    explicit AutoActivateWindowPrivate(AutoActivateWindow *parent);
    ~AutoActivateWindowPrivate();
public:
    AutoActivateWindow *q;
    bool m_run = false;
    QWidget *m_watchedWidget = nullptr;
public: //wayland
    void watchOnWayland(bool on);
public: //x11
    void watchOnX11(bool on);
protected slots:
    void checkWindowOnX11();
private:
    bool initConnect();
    xcb_connection_t *m_x11Con = nullptr;
    xcb_window_t m_rootWin = {0};
    xcb_window_t m_watchedWin = {0};
    QTimer m_checkTimer;
};

#endif // AUTOACTIVATEWINDOW_P_H
