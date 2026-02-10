// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTOACTIVATEWINDOW_P_H
#define AUTOACTIVATEWINDOW_P_H

#include "ddplugin_wallpapersetting_global.h"
#include "autoactivatewindow.h"

#include <QTimer>

#include <xcb/xcb.h>

namespace ddplugin_wallpapersetting {
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
}
#endif // AUTOACTIVATEWINDOW_P_H
