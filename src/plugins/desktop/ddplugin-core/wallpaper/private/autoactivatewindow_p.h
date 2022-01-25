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

#include "dfm_desktop_service_global.h"
#include "autoactivatewindow.h"

#include <QTimer>

#include <xcb/xcb.h>

DSB_D_BEGIN_NAMESPACE
class AutoActivateWindowPrivate : public QObject
{
    Q_OBJECT
public:
    explicit AutoActivateWindowPrivate(AutoActivateWindow *parent);
    ~AutoActivateWindowPrivate();
public:
    AutoActivateWindow *q;
    bool run = false;
    QWidget *watchedWidget = nullptr;
public: //wayland
    void watchOnWayland(bool on);
public: //x11
    void watchOnX11(bool on);
protected slots:
    void checkWindowOnX11();
private:
    bool initConnect();
    xcb_connection_t *x11Con = nullptr;
    xcb_window_t rootWin = {0};
    xcb_window_t watchedWin = {0};
    QTimer checkTimer;
};
DSB_D_END_NAMESPACE
#endif // AUTOACTIVATEWINDOW_P_H
